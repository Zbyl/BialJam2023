
#include "Animation.h"

#include "Utilities.h"

#include "zerrors.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <numeric>


std::tuple< raylib::Vector2, raylib::Texture2D&, raylib::Sound* > Animation::spriteForTime(float animationTime) {
    ZASSERT(animationLength > 0.0f);

    if (!loop && (animationTime >= animationLength))
        return std::tuple< raylib::Vector2, raylib::Texture2D&, raylib::Sound* > { origins.back(), *images.back(), nullptr };

    while (animationTime >= animationLength)
        animationTime -= animationLength;
    float time = 0.0f;
    int i = 0;
    for (; i < std::ssize(images); ++i) {
        time += delays[i];
        if (animationTime < time)
            break;
    }

    return std::tuple< raylib::Vector2, raylib::Texture2D&, raylib::Sound* > { origins[i], *images[i], sounds[i] };
}


void Animation::load(ResourceCache& resourceCache, const std::string& animFile) {
    images.clear();
    origins.clear();
    sounds.clear();
    delays.clear();

    auto jsonText = loadTextFile(animFile);

    auto json = nlohmann::json::parse(jsonText);

    auto basePath = std::filesystem::path(animFile).parent_path();

    for (auto frame : json) {
        auto imagePath = frame["image"].get<std::string>();
        auto originX = frame["origin"]["x"].get<float>();
        auto originY = frame["origin"]["y"].get<float>();
        auto delay = frame["delay"].get<float>();

        std::string soundPath;
        if (frame.contains("sound")) {
            soundPath = frame["sound"].get<std::string>();
        }

        if (soundPath.empty())
            sounds.push_back(nullptr);
        else
            sounds.emplace_back(resourceCache.getSound((basePath / soundPath).string()));

        if (imagePath.empty())
            images.push_back(resourceCache.getEmptyImage());
        else
            images.push_back(resourceCache.getImage((basePath / imagePath).string()));
        origins.emplace_back(originX, originY);
        delays.push_back(delay);
    }

    animationLength = std::accumulate(delays.begin(), delays.end(), 0.0f);
}

void Animation::fromPicture(ResourceCache& resourceCache, const std::string& imageFile, float length) {
    images.clear();
    origins.clear();
    sounds.clear();
    delays.clear();

    images.push_back(resourceCache.getImage(imageFile));
    sounds.push_back(nullptr);
    origins.emplace_back(0.0f, 0.0f);
    delays.push_back(length);
    animationLength = length;
}
