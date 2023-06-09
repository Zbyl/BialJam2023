#pragma once

#include "Animation.h"


#include "raylib-cpp.hpp"


class Game;


enum class PlayerState {
    WALKING,
};

class Player {
private:
    Game& game;

public:
    raylib::Vector2 position;
    PlayerState state;
    float animTime = 0.0f; ///< Time for current animation.
    Animation runAnimation;

public:
    Player(Game& game) : game(game) {
        runAnimation.load("Graphics/Player/player-run.json");
    }

    void update();
};

