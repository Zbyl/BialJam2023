#pragma once

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
};

inline bool isCollider(TileType tile) {
    switch (tile) {
        case TileType::EMPTY: return false;
        case TileType::WALL: return true;
    }
    ZASSERT(false);
}

class Level {
private:
    Game& game;

public:
    raylib::Music music;

private:
    int tileSize = 16;      ///< Tiles are squares of this size.
    int levelWidth;         ///< Width of the level in pixels. Multiples of tileSize.
    int levelHeight;        ///< Width of the level in pixels. Multiples of tileSize.
    std::vector<raylib::Texture2D> backgrounds; ///< Level images drawn before entities.
    std::vector<raylib::Texture2D> foregrounds; ///< Level images drawn after entities.
    std::vector<raylib::Texture2D> paralaxLayers;
    std::vector<raylib::Vector2> paralaxScales;
    std::vector<int8_t> levelData; ///< Level data, where top-left tile is first, bottom-right is last.

    raylib::Vector2 playerStartPosition = { 0.0f, 0.0f };

public:
    Level(Game& game) : game(game) {}

    void load(const std::string& levelFile);

    void startLevel();

    void drawBackground();
    void update();

    std::optional<TileType> getTileRaw(int x, int y) const;
    std::optional<TileType> getTileWorld(raylib::Vector2 worldPosition) const;

    std::tuple<bool, bool, bool, int, raylib::Vector2> collisionDetection(raylib::Rectangle hitBox, raylib::Vector2 velocity);
};
