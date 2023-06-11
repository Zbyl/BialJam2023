#pragma once

#include "Menu.h"
#include "Player.h"
#include "Level.h"
#include "Collectible.h"
#include "Scene.h"

#include "raylib-cpp.hpp"

enum class GameState {
    START_SCREEN,
    LEVEL,
    LEVEL_SUCCESS,
    LEVEL_DIED,
    GAME_SUCCESS,
};

inline std::string to_string(GameState state) {
    switch (state) {
        case GameState::START_SCREEN: return "START_SCREEN";
        case GameState::LEVEL: return "LEVEL";
        case GameState::LEVEL_SUCCESS: return "LEVEL_SUCCESS";
        case GameState::LEVEL_DIED: return "LEVEL_DIED";
        case GameState::GAME_SUCCESS: return "GAME_SUCCESS";
    }
    ZASSERT(false);
}

class Game
{
public:
    const int screenWidth = 1280;
    const int screenHeight = 720;

    raylib::Window window;
    raylib::AudioDevice audioDevice;
    raylib::Gamepad gamepad;
    raylib::Font hudFont;

    GameState gameState = GameState::START_SCREEN;
    float levelTime = 0.0f; ///< In-game time since start of the level, in seconds. Not counting in-menu time.
    float levelTimeDelta = 0.0f; ///< In-game time since start of the last framw, in seconds. Not counting in-menu time.
    bool shouldQuit = false;
    Menu menu;

    std::vector<std::string> levelFiles { "Levels/Level0.json", "Levels/Level1.json", "Levels/Level2.json" };
    int currentLevel = 0;       ///< Current or last level played.
    int totalCollected = 0;     ///< Number of collected collectibles.
    int totalAvailable = 0;     ///< Number of collectibles that were available.
    raylib::Vector2 cameraPosition = { 0, 0 }; ///< Camera position in world coordinates.
    Player player;
    Level level;
    CollectiblePrefab collectiblePrefab;
    Collectible hudCollectible;                 ///< For drawing on HUD.

    Scene startScreen;
    Scene deadScreen;
    Scene levelEndScreen;
    Scene gameEndScreen;

public:
    Game()
        : window(screenWidth, screenHeight, "Kunek Bogus")
        , menu(*this)
        , player(*this)
        , level(*this)
        , collectiblePrefab(*this)
        , hudCollectible(collectiblePrefab)
        , hudFont("Graphics/Fonts/jupiter_crash.png")
        , startScreen(*this)
        , deadScreen(*this)
        , levelEndScreen(*this)
        , gameEndScreen(*this)
    {
        SetTargetFPS(60); // Set our game to run at 60 frames-per-second
        hudCollectible.forHud = true;
        reloadScenes(menu.useFuthark, false);
    }

    void mainLoop();

    raylib::Vector2 worldToScreen(raylib::Vector2 worldPosition) const;
    raylib::Vector2 screenToWorld(raylib::Vector2 screenPosition) const;

    void drawSprite(raylib::Vector2 worldPosition, const raylib::Texture2D& sprite, raylib::Vector2 spriteOrigin, bool horizontalMirror) const;

    void drawHud(bool withTotals);
    void cameraUpdate();
    void restartLevel();
    void restartGame();
    void startLevel(int levelIndex);
    void endLevel(bool died);

    void reloadScenes(bool useFuthark, bool reloadHack);
};
