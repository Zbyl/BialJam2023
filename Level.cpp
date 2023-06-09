
#include "Level.h"

#include "Utilities.h"
#include "Game.h"

#include "zerrors.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <numeric>
#include <sstream>


void Level::load(const std::string& levelFile) {
    auto jsonText = loadTextFile(levelFile);
    auto json = nlohmann::json::parse(jsonText);
    auto basePath = std::filesystem::path(levelFile).parent_path();

    tileSize = json["tileSize"].get<int>();

    auto musicPath = json["music"].get<std::string>();
    music.Load((basePath / musicPath).string());

    for (auto item : json["backgrounds"]) {
        auto imagePath = item.get<std::string>();
        backgrounds.emplace_back((basePath / imagePath).string());
    }

    for (auto item : json["foregrounds"]) {
        auto imagePath = item.get<std::string>();
        foregrounds.emplace_back((basePath / imagePath).string());
    }

    for (auto layer : json["paralaxLayers"]) {
        auto imagePath = layer["image"].get<std::string>();
        auto scaleX = layer["scale"]["x"].get<float>();
        auto scaleY = layer["scale"]["y"].get<float>();

        paralaxLayers.emplace_back((basePath / imagePath).string());
        paralaxScales.emplace_back(scaleX, scaleY);
    }

    auto ldtkDir = basePath / json["ldtkMap"].get<std::string>();
    auto ldtkDataText = loadTextFile((ldtkDir / "data.json").string());
    auto ldtkData = nlohmann::json::parse(ldtkDataText);
    levelWidth = ldtkData["width"].get<int>();
    levelHeight = ldtkData["height"].get<int>();
    ZASSERT(levelWidth % tileSize == 0);
    ZASSERT(levelHeight % tileSize == 0);

    auto intGridText = loadTextFile((ldtkDir / "IntGrid.csv").string());
    std::stringstream intGridStream(intGridText);
    auto levelSize = (levelWidth / tileSize) * (levelHeight / tileSize);
    levelData.resize(levelSize);
    for (int i = 0; i < levelSize; ++i) {
        if (i > 0) {
            char comma;
            intGridStream >> comma;
        }
        int value;
        intGridStream >> value;
        levelData[i] = static_cast<int8_t>(value);
    }
}

void Level::startLevel() {
    music.Play();
    game.player.position = playerStartPosition;
}

void Level::drawBackground() {
    for (int i = 0; i < std::ssize(backgrounds); ++i) {
        backgrounds[i].Draw(game.worldToScreen({ 0.0f, 0.0f }));
    }

    for (int i = 0; i < std::ssize(paralaxLayers); ++i) {
        auto pos = game.cameraPosition * paralaxScales[i];
        //paralaxLayers[i].Draw(game.worldToScreen(pos));
    }
}

void Level::update() {
    for (int i = 0; i < std::ssize(foregrounds); ++i) {
        foregrounds[i].Draw(game.worldToScreen({ 0.0f, 0.0f }));
    }
}
