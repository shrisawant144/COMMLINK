#!/usr/bin/env python3
"""Build, sign, or verify a CommLink Android release from a local Qt and Android SDK."""
import argparse
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import zipfile

from _packaging import PRIVATE_SUFFIXES, ROOT, sha256, source_archive

ABI_MACHINES = {'arm64-v8a': 183, 'x86_64': 62}
BUILD_TOOLS = '36.0.0'
PAGE_SIZE = 16384


def environment(args):
    env = os.environ.copy()
    env.update(JAVA_HOME=str(args.java_home), ANDROID_HOME=str(args.sdk),
               ANDROID_SDK_ROOT=str(args.sdk))
    env['PATH'] = str(args.java_home / 'bin') + os.pathsep + env['PATH']
    return env


def tool(args, name):
    return str(args.sdk / 'build-tools' / BUILD_TOOLS / name)


def build(args):
    build_dir = args.build_dir or ROOT / f'build-android-{args.abi}'
    if build_dir == ROOT or ROOT.is_relative_to(build_dir):
        raise ValueError('Choose a dedicated build directory')
    output = args.output or ROOT / f'dist/android/CommLink-{args.abi}-unsigned.apk'
    package_dir = build_dir / 'commlink-package'
    shutil.copytree(ROOT / 'android', package_dir, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns('source'))
    source_archive(package_dir / 'assets/source/CommLink-Android-source.zip')

    env = environment(args)
    env['GRADLE_USER_HOME'] = str(args.gradle_user_home) if args.gradle_user_home else env.get(
        'GRADLE_USER_HOME', str(Path.home() / '.gradle'))
    options = {
        'QT_HOST_PATH': args.qt_host, 'ANDROID_SDK_ROOT': args.sdk,
        'ANDROID_NDK_ROOT': args.ndk, 'ANDROID_ABI': args.abi, 'ANDROID_PLATFORM': 'android-28',
        'COMMLINK_ANDROID_PACKAGE_SOURCE_DIR': package_dir,
        'COMMLINK_ANDROID_OPENSSL_DIR': args.openssl_dir, 'COMMLINK_QT_MAJOR_VERSION': 6,
        'COMMLINK_MOBILE_UI': 'ON', 'CMAKE_BUILD_TYPE': 'Release', 'BUILD_TESTS': 'OFF',
    }
    command = [str(args.qt / 'bin/qt-cmake'), '-S', str(ROOT), '-B', str(build_dir), '-G', 'Ninja']
    command.extend(f'-D{name}={value}' for name, value in options.items())
    subprocess.run(command, env=env, check=True)
    subprocess.run(['cmake', '--build', str(build_dir), '--target', 'apk', '--parallel', str(args.jobs)],
                   env=env, check=True)
    apk = build_dir / 'src/android-build/build/outputs/apk/release/android-build-release-unsigned.apk'
    output.parent.mkdir(parents=True, exist_ok=True)
    if apk != output:
        shutil.copy2(apk, output)
    print(f'Unsigned APK {output}')


def check_library(data, name, machine):
    if data[:6] != b'\x7fELF\x02\x01':
        raise ValueError(f'Expected little-endian 64-bit ELF in {name}')
    header = struct.unpack_from('<16sHHIQQQIHHHHHH', data)
    if header[2] != machine:
        raise ValueError(f'Wrong instruction set in {name}')
    load_segments = 0
    for index in range(header[10]):
        segment = struct.unpack_from('<IIQQQQQQ', data, header[5] + index * header[9])
        if segment[0] == 1:
            load_segments += 1
            if segment[7] < PAGE_SIZE or (segment[2] - segment[3]) % PAGE_SIZE:
                raise ValueError(f'Missing 16 KB ELF alignment in {name}')
    if not load_segments:
        raise ValueError(f'Missing ELF load segments in {name}')


def verify(args, apk):
    env = environment(args)
    subprocess.run([tool(args, 'apksigner'), 'verify', '--print-certs', str(apk)], env=env, check=True)
    subprocess.run([tool(args, 'zipalign'), '-c', '-P', '16', '4', str(apk)], check=True)
    badging = subprocess.check_output([tool(args, 'aapt'), 'dump', 'badging', str(apk)], text=True)
    for field in ("name='app.commlink.android'", "sdkVersion:'28'", "targetSdkVersion:'35'",
                  "name='android.permission.INTERNET'"):
        if field not in badging:
            raise ValueError(f'Missing APK metadata {field}')
    if 'android.permission.WRITE_EXTERNAL_STORAGE' in badging:
        raise ValueError('APK requests obsolete shared storage permission')
    with zipfile.ZipFile(apk) as archive:
        names = archive.namelist()
        for name in names:
            if Path(name).suffix.lower() in PRIVATE_SUFFIXES or 'password' in Path(name).name.lower():
                raise ValueError(f'Private material in APK: {name}')
        required = ['assets/source/CommLink-Android-source.zip']
        required.extend(f'lib/{args.abi}/lib{name}.so' for name in
                        ('crypto_3', 'ssl_3', f'Qt6Core_{args.abi}', f'Qt6Widgets_{args.abi}'))
        libraries = [name for name in names if name.startswith('lib/') and name.endswith('.so')]
        if any(name not in names for name in required) or not any('qsqlite' in name for name in libraries):
            raise ValueError('APK is missing source, Qt, SQLite, or TLS files')
        for name in libraries:
            if not name.startswith(f'lib/{args.abi}/'):
                raise ValueError(f'Unexpected ABI in {name}')
            check_library(archive.read(name), name, ABI_MACHINES[args.abi])
    print(f'Verified {args.abi} APK {apk}\nSHA256 {sha256(apk)}')


def sign(args):
    output = args.output or ROOT / ('dist/android/CommLink.apk' if args.abi == 'arm64-v8a'
                                   else f'dist/android/CommLink-{args.abi}.apk')
    if output in (args.apk, args.keystore, args.password_file):
        raise ValueError('Choose an output path different from all input files')
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='commlink-sign-', dir=output.parent) as temporary:
        aligned = Path(temporary) / 'aligned.apk'
        signed = Path(temporary) / 'signed.apk'
        subprocess.run([tool(args, 'zipalign'), '-f', '-P', '16', '4', str(args.apk), str(aligned)],
                       check=True)
        subprocess.run([tool(args, 'apksigner'), 'sign', '--ks', str(args.keystore),
                        '--ks-key-alias', args.alias, '--ks-pass', f'file:{args.password_file}',
                        '--out', str(signed), str(aligned)], env=environment(args), check=True)
        verify(args, signed)
        signed.replace(output)
    print(f'Signed APK {output}')


def absolute_path(value):
    return Path(value).expanduser().resolve()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument('--abi', choices=ABI_MACHINES, default='arm64-v8a')
    common.add_argument('--sdk', type=absolute_path, required=True)
    common.add_argument('--java-home', type=absolute_path, required=True)

    build_parser = commands.add_parser('build', parents=[common])
    build_parser.set_defaults(run=build)
    for name in ('qt', 'qt-host', 'ndk', 'openssl-dir'):
        build_parser.add_argument(f'--{name}', type=absolute_path, required=True)
    for name in ('gradle-user-home', 'build-dir', 'output'):
        build_parser.add_argument(f'--{name}', type=absolute_path)
    build_parser.add_argument('--jobs', type=int, default=4)

    sign_parser = commands.add_parser('sign', parents=[common])
    sign_parser.set_defaults(run=sign)
    sign_parser.add_argument('apk', type=absolute_path)
    sign_parser.add_argument('--keystore', type=absolute_path, required=True)
    sign_parser.add_argument('--password-file', type=absolute_path, required=True)
    sign_parser.add_argument('--alias', default='commlink')
    sign_parser.add_argument('--output', type=absolute_path)

    verify_parser = commands.add_parser('verify', parents=[common])
    verify_parser.set_defaults(run=lambda args: verify(args, args.apk))
    verify_parser.add_argument('apk', type=absolute_path)

    args = parser.parse_args()
    try:
        args.run(args)
    except (OSError, ValueError, struct.error, subprocess.CalledProcessError, zipfile.BadZipFile) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
