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
    std::vector<float> delays;

    float animTime = 0.0f;      ///< Time for current animation.
    float sceneDelay = 0.0f;    ///< How much time to wait before buttons will start working.

    raylib::Rectangle menuRectangle = { 0.0f, 0.0f, 1000.0f, 500.0f };

public:
    Scene(Game& game) : game(game) {}

    void startScene(bool forReload = false);
    void endScene();

    bool areAnimationsFinished() const;
    void update();
    void load(const std::string& sceneFile, bool useFuthark, bool reloadHack);
};
