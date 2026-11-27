#!/usr/bin/env python3
"""Wrap a deployed 64-bit Qt Windows application in a portable NSIS launcher."""

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

from _packaging import ROOT, manifest, pe_imports, source_archive


PRIVATE_SUFFIXES = {".jks", ".keystore", ".key", ".p12", ".pfx"}
PRIVATE_HEADER = re.compile(rb"(?m)^-----BEGIN(?: [A-Z0-9]+)* PRIVATE KEY-----")
SYSTEM_DLLS = set((
    "advapi32 bcrypt cabinet cfgmgr32 comctl32 comdlg32 crypt32 cryptbase d3d9 d3d11 d3d12 "
    "dbghelp dnsapi dwmapi dwrite dxgi dxguid gdi32 gdi32full glu32 hid imagehlp imm32 "
    "iphlpapi kernel32 kernelbase mpr msimg32 msvcrt ncrypt netapi32 normaliz ole32 oleacc "
    "oleaut32 opengl32 powrprof propsys psapi rpcrt4 secur32 setupapi shell32 shlwapi "
    "ucrtbase user32 userenv usp10 uxtheme version winhttp wininet winmm winspool "
    "wintrust ws2_32 wtsapi32"
).split())


def check_file(path):
    if path.is_symlink():
        raise ValueError(f"Symlinks cannot be packaged: {path}")
    if not path.is_file():
        raise ValueError(f"Expected a regular file: {path}")
    if path.suffix.lower() in PRIVATE_SUFFIXES or path.name.lower() in {
        "id_rsa", "id_dsa", "id_ecdsa", "id_ed25519",
    }:
        raise ValueError(f"Private signing/key material cannot be packaged: {path}")
    with path.open("rb") as stream:
        if PRIVATE_HEADER.search(stream.read(16384)):
            raise ValueError(f"Private key cannot be packaged: {path}")


def check_tree(directory):
    if directory.is_symlink() or not directory.is_dir():
        raise ValueError(f"Expected a directory without symlinks: {directory}")
    for path in directory.rglob("*"):
        if path.is_symlink():
            raise ValueError(f"Symlinks cannot be packaged: {path}")
        if not path.is_dir():
            check_file(path)


def validate_payload(payload, allow_no_tls):
    files = {path.name.lower(): path for path in payload.iterdir() if path.is_file()}
    application = files.get("commlink.exe")
    if application is None:
        raise ValueError("Payload must contain commlink.exe (case insensitive)")
    versions = [major for major in (5, 6) if f"qt{major}core.dll" in files]
    if len(versions) != 1:
        raise ValueError("Payload root must contain exactly one Qt5Core.dll or Qt6Core.dll")
    major = versions[0]
    required = [f"Qt{major}{module}.dll" for module in (
        "Core", "Gui", "Widgets", "Network", "Sql", "WebSockets",
    )]
    missing = [name for name in required if name.lower() not in files]
    for relative in ("platforms/qwindows.dll", "sqldrivers/qsqlite.dll"):
        if not (payload / relative).is_file():
            missing.append(relative)
    if missing:
        raise ValueError("Missing deployed Qt runtime: " + ", ".join(missing))
    ssl = [path for name, path in files.items() if name.startswith("libssl") and name.endswith(".dll")]
    crypto = [path for name, path in files.items() if name.startswith("libcrypto") and name.endswith(".dll")]
    ssl_versions = {path.name.lower()[len("libssl"):] for path in ssl}
    crypto_versions = {path.name.lower()[len("libcrypto"):] for path in crypto}
    if not allow_no_tls and not ssl_versions.intersection(crypto_versions):
        raise ValueError("TLS DLLs are missing: supply the libssl/libcrypto DLLs matching Qt "
                         "with --openssl-dir, or explicitly use --allow-no-tls")
    binaries = [application] + [path for path in payload.rglob("*")
                                if path.is_file() and path.suffix.lower() == ".dll"]
    for binary in binaries:
        machine, imports = pe_imports(binary)
        if machine != 0x8664:
            raise ValueError(f"Expected an x86-64 Windows executable/DLL: {binary}")
        available = set(files) | {path.name.lower() for path in binary.parent.iterdir() if path.is_file()}
        missing = [name for name in imports if name not in available
                   and Path(name).stem not in SYSTEM_DLLS
                   and not name.startswith(("api-ms-win-", "ext-ms-win-"))]
        if missing:
            raise ValueError(f"Missing DLL dependencies for {binary.name}: {', '.join(sorted(missing))}")
    return application.name


def add_sources(payload):
    # Preflight before the common snapshot helper excludes generated files.
    # The shipped source ZIP can rebuild this package without a Git checkout.
    for name in ("src", "include", "resources", "cmake", "scripts", "android", "tests", "docs", "packaging"):
        if (ROOT / name).exists():
            check_tree(ROOT / name)
    destination = payload / "source"
    destination.mkdir(exist_ok=True)
    source_archive(destination / "CommLink-source.zip")
    shutil.copy2(ROOT / "packaging/windows/commlink-single-file.nsi", destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload-dir", type=Path, required=True,
                        help="folder containing commlink.exe and its deployed Qt runtime")
    parser.add_argument("--output", type=Path, default=ROOT / "dist/windows/CommLink.exe",
                        help="single executable to create (default: dist/windows/CommLink.exe)")
    parser.add_argument("--makensis", default="makensis", help="NSIS 3 makensis executable")
    parser.add_argument("--nsis-dir", type=Path, help="NSIS data directory (sets NSISDIR)")
    parser.add_argument("--windeployqt", help="deploy Qt into the staged copy using this windeployqt")
    parser.add_argument("--openssl-dir", type=Path, help="directory containing matching OpenSSL DLLs")
    parser.add_argument("--allow-no-tls", action="store_true", help="explicitly package without OpenSSL")
    parser.add_argument("--license-file", type=Path, action="append", default=[],
                        help="additional runtime license/notice file (repeatable)")
    parser.add_argument("--icon", type=Path, default=ROOT / "resources/logo/CommLink.ico",
                        help="Windows .ico for the outer launcher (default: repository icon)")
    parser.add_argument("--version", default=(ROOT / "VERSION").read_text().strip())
    args = parser.parse_args()
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:\.\d+)?", args.version):
        raise ValueError("Version must contain three or four numeric components")
    parts = [int(value) for value in args.version.split(".")]
    if any(part > 65535 for part in parts):
        raise ValueError("Version components must be between 0 and 65535")
    win_version = ".".join(map(str, parts + [0] * (4 - len(parts))))
    payload_dir = args.payload_dir.absolute()
    check_tree(payload_dir)  # Check before copying, so symlinks cannot pull in private files.
    output = args.output.resolve()
    if payload_dir.resolve() == output or payload_dir.resolve() in output.parents:
        raise ValueError("Output must be outside the input payload directory")
    if args.openssl_dir:
        check_tree(args.openssl_dir)
    for path in args.license_file + ([args.icon] if args.icon else []):
        check_file(path)
    output.parent.mkdir(parents=True, exist_ok=True)
    # Build beside the output so publishing the completed executable is atomic.
    with tempfile.TemporaryDirectory(prefix="commlink-package-", dir=output.parent) as temporary:
        staging = Path(temporary)
        payload = staging / "payload"
        shutil.copytree(payload_dir, payload)
        if args.windeployqt:
            applications = [path for path in payload.iterdir() if path.name.lower() == "commlink.exe"]
            if len(applications) != 1:
                raise ValueError("Payload must contain commlink.exe before running windeployqt")
            subprocess.run([args.windeployqt, "--release", "--compiler-runtime", "--no-translations",
                            str(applications[0])], check=True)
        if args.openssl_dir:
            for path in args.openssl_dir.iterdir():
                if path.is_file() and path.suffix.lower() == ".dll" and path.name.lower().startswith(("libssl", "libcrypto")):
                    shutil.copy2(path, payload / path.name)
        check_tree(payload)
        app_exe = validate_payload(payload, args.allow_no_tls)
        (payload / "qt.conf").write_text("[Paths]\nPrefix=.\nPlugins=.\n", encoding="utf-8")
        shutil.copy2(ROOT / "LICENSE", payload / "LICENSE.txt")
        licenses = payload / "licenses"
        licenses.mkdir(exist_ok=True)
        for path in args.license_file:
            shutil.copy2(path, licenses / path.name)
        add_sources(payload)
        (payload / "SINGLE-FILE.txt").write_text(
            "Share the outer CommLink.exe by itself. It extracts its embedded runtime into a\n"
            "unique temporary directory and removes it after the app exits normally.\n"
            "No installation or administrator access is needed. Settings and history use\n"
            "the Windows user profile. Source is in source/CommLink-source.zip.\n",
            encoding="utf-8")
        manifest(payload)
        environment = os.environ.copy()
        if args.nsis_dir:
            environment["NSISDIR"] = str(args.nsis_dir.resolve())
        # Native makensis uses / switches; POSIX builds use - switches.
        prefix = "/" if os.name == "nt" else "-"
        definitions = {
            "PAYLOAD_DIR": payload, "OUTPUT_EXE": staging / "CommLink.exe",
            "APP_EXE": app_exe, "APP_VERSION": args.version, "WIN_VERSION": win_version,
        }
        if args.icon:
            definitions["APP_ICON"] = args.icon.resolve()
        command = [args.makensis, prefix + "NOCONFIG", prefix + "V3"]
        command.extend(f"{prefix}D{name}={value}" for name, value in definitions.items())
        command.append(str(ROOT / "packaging/windows/commlink-single-file.nsi"))
        subprocess.run(command, check=True, env=environment)
        os.replace(staging / "CommLink.exe", output)
    print(f"Single-file executable: {output} ({output.stat().st_size:,} bytes)")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Packaging failed: {error}", file=sys.stderr)
        sys.exit(1)
