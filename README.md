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


# Building for the Web

TODO...

# Used assets

TODO...

- Graphics: https://ansimuz.itch.io/sunny-land-pixel-game-art
- JUPITER_CRASH FONT designed by Brian Kent (AEnigma)

Tilesets:
https://itch.io/game-assets/tag-tileset
https://adamatomic.itch.io/gallet-city


# Todo

- Web build.
- Improve collision detection.
- Fix Camera Window.
- Collectible should take player hitbox into consideration.
- Add sounds in menu.
- More punchy end game screen.
- Tweak first level, maybe last as well.
- Make fire animated.
- Move from Raylib to SDL.
