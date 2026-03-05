# Full SDL2 C++ Port + Android ARM64

This is now a full playable SDL2 port flow (not only core logic):
- Menu state (`Play`, `Play Custom Level Pack`).
- Gameplay update loop with collisions, bosses, fireballs, checkpoint, HP/respawn.
- Debug level selector (`I` + `E`, then `UP/DOWN`, `ENTER`).
- Mouse drag platforms.
- HUD + in-game messages.

Code entrypoint:
- `sdl2_full_port.cpp`

## Controls
- Move: `A/D` or `Left/Right`
- Jump: `W`, `Up`, or `Space`
- Back to menu: `Esc`
- Toggle debug selector: hold `I` then press `E`
- Custom pack load: `C` (loads from `./custom_levels` directory)

## Desktop build (SDL2)
```bash
cmake -S cpp_port -B cpp_port/build
cmake --build cpp_port/build --config Release
```

Run:
```powershell
.\cpp_port\build\Release\xplataformer_sdl2.exe
```

## Android ARM64 APK with SDL2

Use SDL2's Android project template and drop this source in the JNI/CMake layer.

1. Create Android project from SDL2 template or integrate SDL2 AAR/native setup.
2. Put these files in `app/src/main/cpp/`:
   - `game_core.hpp`
   - `game_core.cpp`
   - `sdl2_full_port.cpp`
3. `app/src/main/cpp/CMakeLists.txt` example:
```cmake
cmake_minimum_required(VERSION 3.22)
project(xplataformer_sdl_android LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(main SHARED
    sdl2_full_port.cpp
    game_core.cpp
)

# SDL2 is usually provided by the SDL Android template build scripts.
# If needed, link with the imported SDL2 target from your template.
```
4. `app/build.gradle` (ARM64 only):
```gradle
android {
    defaultConfig {
        ndk {
            abiFilters "arm64-v8a"
        }
        externalNativeBuild {
            cmake {
                cppFlags "-std=c++17 -O2"
                abiFilters "arm64-v8a"
            }
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
5. Place `levels/` under Android assets and ensure your startup working directory can access them (or copy assets to internal storage at startup).
6. Build release APK:
```bash
./gradlew assembleRelease
```
7. Verify APK native ABI:
```bash
aapt dump badging app-release.apk | grep native-code
```
Expect: `native-code: 'arm64-v8a'`.

## Current limitation versus Java original
- Java Swing file chooser and `.xlevel` zip package picker are replaced by SDL-friendly directory loading (`./custom_levels`).
- If you want true `.xlevel` zip loading in C++, add a zip library (`miniz`, `libzip`) and wire it into `GameCore`.
