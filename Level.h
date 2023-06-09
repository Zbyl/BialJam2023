#pragma once

#include "Collectible.h"

#include "zerrors.h"

#include "raylib-cpp.hpp"

#include <vector>
#include <cstdint>
#include <optional>
#include <tuple>


class Game;

enum class TileType {
    EMPTY = 0,
    WALL = 1,
    LAVA = 2,
    INVISIBLE_WALL = 3,
};

inline bool isCollider(TileType tile) {
    switch (tile) {
        case TileType::EMPTY: return false;
        case TileType::WALL: return true;
        case TileType::LAVA: return true;
        case TileType::INVISIBLE_WALL: return true;
    }
    ZASSERT(false);
}

class Level {
private:
    Game& game;

public:
    raylib::Music music;

public:
    int tileSize = 16;      ///< Tiles are squares of this size.
    int levelWidth;         ///< Width of the level in pixels. Multiples of tileSize.
    int levelHeight;        ///< Width of the level in pixels. Multiples of tileSize.
    std::u32string levelDescription;            ///< Desription of the level.

private:
    std::vector<raylib::Texture2D> backgrounds; ///< Level images drawn before entities.
    std::vector<raylib::Texture2D> foregrounds; ///< Level images drawn after entities.
    std::vector<raylib::Texture2D> paralaxLayers;
    std::vector<raylib::Vector2> paralaxScales;
    std::vector<int8_t> levelData; ///< Level data, where top-left tile is first, bottom-right is last.
    std::vector<float> paralaxHaxxorOffsets; ///< Add to paralax y, cause no time to fix...

    std::vector<Collectible> collectibles;

public:
    raylib::Vector2 playerStartPosition = { 0.0f, 0.0f };
    raylib::Rectangle levelExit = { 0.0f, 0.0f, 0.0f, 0.0f };
    raylib::Rectangle levelExitDoor = { 0.0f, 0.0f, 0.0f, 0.0f };
    raylib::Rectangle furharkBubble = { 0.0f, 0.0f, 0.0f, 0.0f };
    raylib::Rectangle furharkTrigger = { 0.0f, 0.0f, 0.0f, 0.0f };

    bool showFuthark = false;
    float showFutharkStartTime = 0.0f;
    bool levelEnding = false;           ///< True if level end sequence plays.
    float levelEndingStartTime = 0.0f;  ///< When level ending started.
    float extraLevelEndDelay = 0.0f;
    bool levelEndingByDeath = false;
    Animation exitDoorAnimation;
    Animation futharkAnimation;

public:
    Level(Game& game) : game(game) {}

    void load(const std::string& levelFile);

    void startLevel();
    void setShowFuthark();
    void setLevelEnding(bool death);
    bool isLevelEnding() const { return levelEnding; }
    bool hasLevelEnded() const;
    void endLevel();

    void drawBackground();
    void update();

    std::optional<TileType> getTileRaw(int x, int y) const;
    std::optional<TileType> getTileWorld(raylib::Vector2 worldPosition) const;

    std::tuple<bool, bool, bool, int, raylib::Vector2> collisionDetection(raylib::Rectangle hitBox, raylib::Vector2 velocity);

    /// @returns [ collectedCount, totalCount ]
    std::tuple<int, int> getCollectibleStats() const;
};
