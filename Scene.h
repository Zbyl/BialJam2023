#pragma once

#include "Animation.h"

#include "raylib-cpp.hpp"

#include <tuple>
#include <vector>

class Game;

class Scene
{
private:
    Game& game;

public:
    raylib::Music music;

    std::vector<Animation> animations;
    std::vector<raylib::Vector2> positions;

    float animTime = 0.0f;      ///< Time for current animation.

public:
    Scene(Game& game, const std::string& sceneFile)
        : game(game)
    {
        load(sceneFile);
    }

    void startScene() {
        animTime = 0.0f;
        music.Seek(0);
        music.Play();
    }

    void endScene() {
        music.Stop();
    }

    bool areAnimationsFinished() const;
    void update();
    void load(const std::string& sceneFile);
};
