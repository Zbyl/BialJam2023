#pragma once

#include "raylib-cpp.hpp"

#include <memory>
#include <unordered_map>


class ResourceCache
{
public:
    raylib::Texture2D emptyImage;
    std::unordered_map<std::string, std::unique_ptr<raylib::Texture2D>> imageCache; ///< Cache of images, so that we only load once.
    std::unordered_map<std::string, std::unique_ptr<raylib::Sound>> soundCache; ///< Cache of sounds, so that we only load once.

public:
    raylib::Texture2D* getEmptyImage() { return &emptyImage; }
    raylib::Texture2D* getImage(const std::string& fileName);
    raylib::Sound* getSound(const std::string& fileName);
};
