#pragma once

#include "ResourceCache.h"

#include "raylib-cpp.hpp"

#include <tuple>
#include <vector>


class Animation {
private:
    std::vector<raylib::Texture2D*> images; ///< Empty image to show empty image.
    std::vector<raylib::Sound*> sounds;     ///< Nullptr for no sound.
    std::vector<raylib::Vector2> origins;
    std::vector<float> delays;      ///< How long to display given frame (in seconds).
    float animationLength = 0.0f;   ///< How long is the animation (in seconds).

public:
    bool loop = true;               ///< True to loop.

public:
    std::tuple< raylib::Vector2, raylib::Texture2D&, raylib::Sound* > spriteForTime(float animationTime);

    float getAnimationLength() const { return animationLength; }

    void load(ResourceCache& resourceCache, const std::string& animFile);
    void fromPicture(ResourceCache& resourceCache, const std::string& imageFile, float length = 1.0f);
};

