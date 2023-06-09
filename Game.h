#pragma once

#include "Menu.h"
#include "Player.h"
#include "Level.h"

#include "raylib-cpp.hpp"

class Game
{
public:
    const int screenWidth = 1280;
    const int screenHeight = 720;

    raylib::Window window;
    raylib::AudioDevice audioDevice;
    raylib::Gamepad gamepad;

    raylib::Sound sfx;

    float levelTime = 0.0f; ///< In-game time since start of the level, in seconds. Not counting in-menu time.
    float levelTimeDelta = 0.0f; ///< In-game time since start of the last framw, in seconds. Not counting in-menu time.
    bool shouldQuit = false;
    Menu menu;

    raylib::Vector2 cameraPosition = { 0, 0 }; ///< Camera position in world coordinates.
    Player player;
    Level level;

public:
    Game()
        : window(screenWidth, screenHeight, "KunekBogus")
        , sfx("Sounds/663831__efindlay__springy-jump.wav")
        , menu(*this)
        , player(*this)
        , level(*this)
    {
        SetTargetFPS(60); // Set our game to run at 60 frames-per-second
        level.load("Levels/Level0.json");
    }

    void mainLoop();

    raylib::Vector2 worldToScreen(raylib::Vector2 worldPosition);
    raylib::Vector2 screenToWorld(raylib::Vector2 screenPosition);
};
