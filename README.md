# Kunek Boguś

# Building for Windows

1. Install Visual Studio 2022
2. Install CMake
3. Run:
    ```
    cd Build

    git clone https://github.com/microsoft/vcpkg
    .\vcpkg\bootstrap-vcpkg.bat
   
    .\vcpkg\vcpkg install raylib:x64-windows
    .\vcpkg\vcpkg install nlohmann-json:x64-windows

    git clone https://github.com/raysan5/raygui.git
    git clone https://github.com/RobLoach/raylib-cpp.git
    ```
4. Generate VS project files:
   ```
   build.bat
   ```
5. Open `Build/VSProject/RayGame.sln`, compile and run!


# Building for the Web on Windows

Official guide is in raylib README.md.

## Prerequisites:

1. Python and Git must be installed.
2. Install Emscripten:
   ```
   cd Build
   git clone https://github.com/emscripten-core/emsdk.git
   emsdk\emsdk install latest
   ```
3. Unpack `ninja.exe` into `Build` (https://github.com/ninja-build/ninja/releases).
4. For Debug builds to work hack Raylib to NOT use sanitizers in Debug:  
   edit `Build\vcpkg\ports\raylib\portfile.cmake` and set `DEBUG_ENABLE_SANITIZERS` to `OFF`.
3. Install dependencies for Emscripten:
   ```
   .\vcpkg\vcpkg install raylib:wasm32-emscripten
   .\vcpkg\vcpkg install nlohmann-json:wasm32-emscripten
   ```

## Build

1. Activate emscripten and add `ninja` to `PATH`:
   ```
   cd Build
   emsdk\emsdk activate latest
   set PATH=%CD%;%PATH%
   ```
2. Run CMake and build:
   ```
   build-web.bat
   cmake --build WebProject
   ```
   
## Run

1. Serve files:
    ```
    cd Build
    python -m http.server 8080 --directory WebProject
    ```
2. Open in browser: http://localhost:8080/RayGame.html  
   Note: works in Chrome, sometimes have issues in Firefox. Chrome is more reliable.
3. Click on the game to enable sound.
4. Go to fullscreen for better experience.


## Debug

1. Remeber to hack raylib to not use sanitizers in Debug (otherwise linking takes forever, and build doesn't run - but maybe it does now?).
2. Install WASM debugging plugin: https://developer.chrome.com/blog/wasm-debugging-2020/
3. Change in `build-web.bat` `CMAKE_BUILD_TYPE` to `Debug`.
   
   Stop sounds at level loads.
   Add keyboard support.
   

# Used assets

- Graphics and Music: https://ansimuz.itch.io/sunny-land-pixel-game-art
- Graphics (Viking): https://pitiit.itch.io/free-2d-fantasy-platformer-asset-pack
- JUPITER_CRASH FONT designed by Brian Kent (AEnigma), and other fonts from Raylib.
- Music from Pixabay (free to use):
    - https://pixabay.com/music/acid-jazz-dramatic-music-hip-hop-background-jazz-music-beat-black-amp-white-148451/
    - https://pixabay.com/music/upbeat-catch-it-117676/
    - https://pixabay.com/pl/music/muzyka-pop-focus-152819/
    - https://pixabay.com/pl/music/grupa-akustyczna-forest-lullaby-110624/
    - https://pixabay.com/pl/music/zbiorowy-inspiring-chill-152901/
- Music from DL Sounds:
    - https://www.dl-sounds.com/royalty-free/oniku-loop2/


# Todo

- Improve collision detection.
- Fix Camera Window.
- Collectible should take player hitbox into consideration.
- Add sounds in menu.
- More punchy end game screen.
- Tweak first level, maybe last as well.
- Make fire animated.
- Move from Raylib to SDL.
- To futhark moves menu to start screen position.


# Possible LDtk improvements

- separate AutoLayer doesn't work - rule wizard/editor? doesn't see the values from IntGrid
- rule editor: add Wall OR OtherWall OR OtherSomething and NOT Something else.
- rule editor: add rule: every second one, but starting from first in blob, not from level start.
- rule editor: add code rules
- rule editor: add blob focused rules - blob outline, blob inside, etc.
- add collision meshes to sprites, add "merge collision meshes" option.
