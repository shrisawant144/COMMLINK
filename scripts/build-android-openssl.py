#!/usr/bin/env python3
"""Build Qt-compatible OpenSSL 3 libraries from a trusted local source tree.

Requires Python 3.9+, Perl, GNU make, patchelf and an installed Android NDK.
No sources or tools are downloaded. Build products stay outside the source tree.
"""

import argparse
import os
from pathlib import Path
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile


TARGETS = {"arm64-v8a": ("android-arm64", 183), "x86_64": ("android-x86_64", 62)}
PAGE_SIZE = 16384
REPO = Path(__file__).resolve().parent.parent


def run(command, *, cwd=None, env=None, capture=False):
    print("+ " + shlex.join(str(part) for part in command), flush=True)
    return subprocess.run(
        [str(part) for part in command], cwd=cwd, env=env, check=True,
        text=True, stdout=subprocess.PIPE if capture else None,
    ).stdout


def find_tool(value):
    found = shutil.which(str(value))
    if not found:
        raise ValueError(f"Required executable not found: {value}")
    return Path(found).resolve()


def verify_elf(path, machine):
    """Reject wrong-ABI or 4 KB-only output, including after patchelf edits."""
    data = path.read_bytes()
    if len(data) < 64 or data[:6] != b"\x7fELF\x02\x01":
        raise ValueError(f"Expected a little-endian 64-bit ELF library: {path}")
    header = struct.unpack_from("<16sHHIQQQIHHHHHH", data)
    if header[1] != 3 or header[2] != machine:
        raise ValueError(f"Unexpected ELF architecture/type: {path}")
    load_count = 0
    for index in range(header[10]):
        offset = header[5] + index * header[9]
        if header[9] < 56 or offset + 56 > len(data):
            raise ValueError(f"Invalid ELF program headers: {path}")
        segment = struct.unpack_from("<IIQQQQQQ", data, offset)
        if segment[0] == 1:  # PT_LOAD
            load_count += 1
            if segment[7] < PAGE_SIZE or (segment[2] - segment[3]) % PAGE_SIZE:
                raise ValueError(f"ELF load segment lacks 16 KB alignment: {path}")
    if not load_count:
        raise ValueError(f"ELF library has no load segments: {path}")


def positive_number(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return number


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path,
                        help="trusted, clean extracted OpenSSL 3 source directory")
    parser.add_argument("--ndk", required=True, type=Path, help="installed Android NDK directory")
    parser.add_argument("--abi", required=True, choices=TARGETS)
    parser.add_argument("--output", required=True, type=Path,
                        help="destination for libcrypto_3.so and libssl_3.so")
    parser.add_argument("--build-dir", type=Path,
                        help="out-of-source build directory (default: build/openssl/ABI)")
    parser.add_argument("--host-tag", help="NDK prebuilt host directory, if more than one is installed")
    parser.add_argument("--patchelf", default="patchelf", help="patchelf executable or path")
    parser.add_argument("--jobs", type=positive_number, default=min(4, os.cpu_count() or 1))
    args = parser.parse_args(argv)

    source = args.source.expanduser().resolve()
    ndk = args.ndk.expanduser().resolve()
    output = args.output.expanduser().resolve()
    build = (args.build_dir or REPO / "build" / "openssl" / args.abi).expanduser().resolve()
    if not (source / "Configure").is_file():
        parser.error("--source must contain the OpenSSL Configure script")
    version = source / "VERSION.dat"
    if not version.is_file() or not re.search(r"^MAJOR=3\s*$", version.read_text(), re.MULTILINE):
        parser.error("--source must be an OpenSSL 3 source tree")
    if (source / "configdata.pm").exists():
        parser.error("--source is configured in-place; use a fresh extraction for out-of-source builds")
    if source == build or source in build.parents or source == output or source in output.parents:
        parser.error("--build-dir and --output must be outside the OpenSSL source tree")
    if not (ndk / "source.properties").is_file():
        parser.error("--ndk must be an installed Android NDK")

    prebuilt = ndk / "toolchains" / "llvm" / "prebuilt"
    hosts = sorted(path for path in prebuilt.glob("*") if (path / "bin" / "clang").is_file())
    if args.host_tag:
        hosts = [path for path in hosts if path.name == args.host_tag]
    if len(hosts) != 1:
        parser.error("select one installed NDK prebuilt directory using --host-tag")
    tools = {name: find_tool(name) for name in ("perl", "make")}
    patchelf = find_tool(args.patchelf)
    environment = os.environ.copy()
    environment["ANDROID_NDK_ROOT"] = str(ndk)
    environment["PATH"] = str(hosts[0] / "bin") + os.pathsep + environment.get("PATH", "")
    build.mkdir(parents=True, exist_ok=True)
    target, machine = TARGETS[args.abi]
    run([tools["perl"], source / "Configure", "shared", target, "no-tests",
         "-D__ANDROID_API__=28", "-Wl,-z,max-page-size=16384"], cwd=build, env=environment)
    run([tools["make"], f"-j{args.jobs}", "SHLIB_VERSION_NUMBER=", "build_libs"],
        cwd=build, env=environment)

    # Patch temporary copies so a failed build/patch leaves existing output intact.
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".openssl-", dir=output) as temporary:
        staged = Path(temporary)
        for name in ("crypto", "ssl"):
            shutil.copy2(build / f"lib{name}.so", staged / f"lib{name}_3.so")
        ssl = staged / "libssl_3.so"
        dependencies = run([patchelf, "--print-needed", ssl], capture=True).splitlines()
        crypto_dependencies = [name for name in dependencies
                               if re.fullmatch(r"libcrypto\.so(?:\.\d+)*", name)]
        if len(crypto_dependencies) != 1:
            raise ValueError("Expected exactly one OpenSSL crypto dependency in libssl.so")
        # Keep replace-needed and set-soname separate: older patchelf versions
        # corrupt the dynamic section when both operations share one invocation.
        run([patchelf, "--page-size", str(PAGE_SIZE), "--replace-needed",
             crypto_dependencies[0], "libcrypto_3.so", ssl])
        for name in ("crypto", "ssl"):
            library = staged / f"lib{name}_3.so"
            run([patchelf, "--page-size", str(PAGE_SIZE), "--set-soname", library.name, library])
            verify_elf(library, machine)
            soname = run([patchelf, "--print-soname", library], capture=True).strip()
            if soname != library.name:
                raise ValueError(f"Incorrect SONAME after patching: {library}")
        if "libcrypto_3.so" not in run([patchelf, "--print-needed", ssl], capture=True).splitlines():
            raise ValueError("libssl_3.so does not reference libcrypto_3.so")
        for name in ("crypto", "ssl"):
            library = staged / f"lib{name}_3.so"
            os.replace(library, output / library.name)
    print(f"Built OpenSSL 3 for {args.abi} (Android API 28, 16 KB pages): {output}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
