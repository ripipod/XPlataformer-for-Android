# C++ port notes (from `javamaincode.java`)

This folder contains a gameplay-core translation from Java to C++17.

## What is translated
- Level format parser (`name`, `start`, `platform`, `movingplatform`, `dragplatform`, `invisibleplatform`, `boss`, `killer`, `goal`, `checkpoint`).
- Physics loop and platform collision behavior.
- Boss tired/attack/fireball logic.
- Player HP, invulnerability, respawn, checkpoint, attempt counting.
- Goal progression and level looping.
- Draggable platform state.

## What is intentionally not included here
- Swing UI/menu rendering.
- Java `JFileChooser`-based file picking.
- `.xlevel` zip package loading (can be added with `miniz` or `libzip`).

## Desktop sanity build
```bash
cmake -S cpp_port -B cpp_port/build
cmake --build cpp_port/build --config Release
```

Run (Windows example):
```powershell
.\cpp_port\build\Release\xplataformer_headless.exe levels
```

## Android ARM64 APK guide (NDK + CMake)

You said you will handle compiling, so here is a direct setup:

1. Create a standard Android app project (Kotlin/Java).
2. Add native module files under `app/src/main/cpp/`:
   - `game_core.hpp`
   - `game_core.cpp`
   - your renderer bridge (`native-lib.cpp`) that calls `GameCore::update`.
3. In `app/build.gradle`, set:
```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                cppFlags "-std=c++17 -O2"
                abiFilters "arm64-v8a"
            }
        }
        ndk {
            abiFilters "arm64-v8a"
        }
    }

    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.22.1"
        }
    }
}
```
4. `app/src/main/cpp/CMakeLists.txt` should build a shared lib:
```cmake
cmake_minimum_required(VERSION 3.22)
project(xplataformer_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
add_library(xplataformer SHARED
    native-lib.cpp
    game_core.cpp
)

find_library(log-lib log)
target_link_libraries(xplataformer ${log-lib})
```
5. Put your level files in Android assets (for example `app/src/main/assets/levels/`).
6. In Java/Kotlin, load native lib and pass input each frame:
   - `System.loadLibrary("xplataformer")`
   - map touch/buttons -> `InputState`
   - call native update with `dt`.
7. Build release APK for ARM64:
```bash
./gradlew assembleRelease
```
8. Verify APK includes only ARM64:
```bash
aapt dump badging app-release.apk | grep native-code
```
Expected output includes `native-code: 'arm64-v8a'`.

## Integration tip
Use a `SurfaceView`/`GameView` or SDL2 bridge for rendering. The gameplay model in `GameCore` is renderer-agnostic, so all drawing can be separate.
