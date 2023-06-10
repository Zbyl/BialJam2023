#pragma once

#include "raylib-cpp.hpp"

#include <tuple>
#include <vector>
#include <optional>


class Animation {
private:
    std::vector<raylib::Texture2D> images; ///< Empty string to show empty image.
    std::vector<std::optional<raylib::Sound>> sounds; ///< Empty string for no sound.
    std::vector<raylib::Vector2> origins;
    std::vector<float> delays;      ///< How long to display given frame (in seconds).
    float animationLength = 0.0f;   ///< How long is the animation (in seconds).
    bool loop = true;               ///< True to loop.

public:
    std::tuple< raylib::Vector2, raylib::Texture2D&, raylib::Sound* > spriteForTime(float animationTime);

    float getAnimationLength() const { return animationLength; }

    void load(const std::string& animFile);
    void fromPicture(const std::string& imageFile, float length = 1.0f);
};

