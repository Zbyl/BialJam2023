
#include "Animation.h"

#include "Utilities.h"

#include "zerrors.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <numeric>


std::tuple< raylib::Vector2, raylib::Texture2D& > Animation::spriteForTime(float animationTime) {
    ZASSERT(animationLength > 0.0f);
    while (animationTime >= animationLength)
        animationTime -= animationLength;
    float time = 0.0f;
    int i = 0;
    for (; i < std::ssize(images); ++i) {
        time += delays[i];
        if (animationTime < time)
            break;
    }

    return std::tuple< raylib::Vector2, raylib::Texture2D& > { origins[i], images[i] };
}


void Animation::load(const std::string& animFile) {
    auto jsonText = loadTextFile(animFile);

    auto json = nlohmann::json::parse(jsonText);

    auto basePath = std::filesystem::path(animFile).parent_path();

    for (auto frame : json) {
        auto imagePath = frame["image"].get<std::string>();
        auto originX = frame["origin"]["x"].get<float>();
        auto originY = frame["origin"]["y"].get<float>();
        auto delay = frame["delay"].get<float>();

        images.emplace_back((basePath / imagePath).string());
        origins.emplace_back(originX, originY);
        delays.push_back(delay);
    }

    animationLength = std::accumulate(delays.begin(), delays.end(), 0.0f);
}
