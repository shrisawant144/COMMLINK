# Building CommLink for Ubuntu, Windows, and Android

Run these commands from the repository root. Build outputs go into local build and distribution directories.

## Ubuntu

Install CMake, a C++17 compiler, and the matching Qt Core, Widgets, Network, Sql, WebSockets, and Test development packages. Install the matching Qt SQLite plugin to use message history. Use `-DCOMMLINK_QT_MAJOR_VERSION=5` for Qt 5.15 or `=6` for Qt 6.

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel 4
./build/release/bin/commlink
```

## Windows single-file EXE

Use a 64-bit Qt and MinGW kit that match each other. The packaging script requires Python 3.9+ and NSIS 3.08 with the amd64 Unicode target. On Windows, point CMake at the Qt kit and select its matching C++ compiler. To cross-compile with Qt 5 on Ubuntu, install the Linux Qt 5 code generators and use the MinGW toolchain file.

```sh
cmake -S . -B build-windows -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/toolchains/mingw-w64.cmake" \
  -DCOMMLINK_MINGW_ROOT="$MINGW_ROOT" \
  -DCOMMLINK_WINDOWS_QT_ROOT="$WINDOWS_QT_ROOT" \
  -DCOMMLINK_HOST_QT_BIN="$HOST_QT_BIN" \
  -DCOMMLINK_QT_MAJOR_VERSION=5 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build-windows --parallel 4
```

Place `build-windows/bin/commlink.exe` in a dedicated deployment directory. On Windows, run that kit's `windeployqt --release --compiler-runtime commlink.exe` from the deployment directory, or pass the full path to `windeployqt.exe` with `--windeployqt` when packaging there. On Linux, supply an already deployed directory from the same kit, with the Qt and MinGW DLLs alongside the executable and Qt plugins under `platforms/` and `sqldrivers/`.

Supply the matching x64 OpenSSL runtime DLLs for HTTPS and WSS, and include the required notices and corresponding source for any third-party runtimes you distribute. For a Qt 6 deployment, include its TLS backend plugin in `tls/`.

```sh
python3 scripts/package-windows.py --payload-dir "$WINDOWS_DEPLOY_DIR" \
  --openssl-dir "$WINDOWS_OPENSSL_DIR"
```

Share `dist/windows/CommLink.exe`. It unpacks its bundled runtime into a temporary directory, runs CommLink as the current user, and removes the temporary files after the app closes. Recipients can run it without installing Qt.

## Android phone APK

Use Python 3.9+, CMake 3.22+, Ninja, JDK 17, the matching Qt 6.10.3 Android and desktop host kits, Android SDK platform 36, Build Tools 36.0.0, and NDK 27.2.12479018. The ARM64 build runs on supported 64-bit Android phones with Android 9 or newer. For an x86-64 emulator, select the Qt Android x86-64 kit and pass `--abi x86_64` to each command.

Use ABI-matched Android OpenSSL 3 libraries named `libcrypto_3.so` and `libssl_3.so`, with 16 KB page alignment. To build them from a fresh OpenSSL 3 source extraction, use `make`, Perl, and `patchelf` with the following helper.

```sh
python3 scripts/build-android-openssl.py --source "$OPENSSL_SOURCE" \
  --ndk "$ANDROID_NDK" --abi arm64-v8a --output build-android-openssl/arm64-v8a
```

Set `ANDROID_QT` to the ABI-specific Qt kit, `HOST_QT` to the desktop kit of the same Qt version, and `JAVA_HOME` to JDK 17.

```sh
python3 scripts/android.py build --qt "$ANDROID_QT" --qt-host "$HOST_QT" \
  --sdk "$ANDROID_SDK" --ndk "$ANDROID_NDK" --java-home "$JAVA_HOME" \
  --openssl-dir build-android-openssl/arm64-v8a
```

Keep the signing key and password file outside the repository. Use the same key for updates to an installed APK. A new distributor can create a key with `keytool -genkeypair -storetype PKCS12 -keystore /private/path/commlink.p12 -alias commlink -keyalg RSA -keysize 3072 -validity 10000` and store its password in a private file.

```sh
python3 scripts/android.py sign dist/android/CommLink-arm64-v8a-unsigned.apk \
  --sdk "$ANDROID_SDK" --java-home "$JAVA_HOME" \
  --keystore "$COMMLINK_KEYSTORE" --password-file "$COMMLINK_PASSWORD_FILE"
python3 scripts/android.py verify dist/android/CommLink.apk \
  --sdk "$ANDROID_SDK" --java-home "$JAVA_HOME"
```

Share the signed `dist/android/CommLink.apk`. To install it on a test phone with USB debugging enabled, run `adb install -r dist/android/CommLink.apk`. The packaging script verifies the signature, ABI, page alignment, and bundled SQLite and TLS libraries.

Use `-DCOMMLINK_QT_MAJOR_VERSION=6 -DCOMMLINK_MOBILE_UI=ON` to build the phone layout on desktop for development. Android builds enable it by default.
