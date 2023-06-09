#pragma once

#include "raylib-cpp.hpp"

#include <tuple>
#include <vector>


class Animation {
private:
    std::vector<raylib::Texture2D> images;
    std::vector<raylib::Vector2> origins;
    std::vector<float> delays;  ///< How long to display given frame (in seconds).
    float animationLength = 0.0f; ///< How long is the animation (in seconds).

public:
    std::tuple< raylib::Vector2, raylib::Texture2D& >  spriteForTime(float animationTime);

    void load(const std::string& animFile);
};

