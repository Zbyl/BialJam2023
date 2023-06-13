#pragma once

#include "Animation.h"


#include "raylib-cpp.hpp"
#include "zerrors.h"


class Game;


class CollectiblePrefab {
public:
    Game& game;

public:
    raylib::Rectangle hitbox;
    Animation wiggleAnimation;
    raylib::Sound collectSfx;

public:
    CollectiblePrefab(Game& game);

    void load();
};

class Collectible {
private:
    CollectiblePrefab& collectiblePrefab;

public:
    raylib::Vector2 position = { 0.0f, 0.0f };
    float animTime = 0.0f;      ///< Time for current animation.
    bool collected = false;     ///< True if it was already collected.
    bool forHud = false;        ///< If True collisions don't count, position is screen pos (as opposed to world pos). Doesn't count towards level collectibles.

public:
    Collectible(CollectiblePrefab& collectiblePrefab)
        : collectiblePrefab(collectiblePrefab)
    {
    }

    void update();
};
