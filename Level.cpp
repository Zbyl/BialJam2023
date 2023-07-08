
#include "Level.h"

#include "Utilities.h"
#include "Game.h"

#include "zerrors.h"

#include "nlohmann/json.hpp"

#include "raylib.h"

#include <filesystem>
#include <numeric>
#include <sstream>
#include <algorithm>


void Level::load(const std::string& levelFile) {
    exitDoorAnimation.load(game.resourceCache, "Graphics/Door/door-open-close.json"); // Here so that we can iterate on it more easily, as it reloads.
    exitDoorAnimation.loop = false;
    futharkAnimation.load(game.resourceCache, "Graphics/Viking/SpeechBubble.json"); // Here so that we can iterate on it more easily, as it reloads.
    futharkAnimation.loop = false;

    backgrounds.clear();
    foregrounds.clear();
    paralaxLayers.clear();
    paralaxScales.clear();
    paralaxHaxxorOffsets.clear();
    levelData.clear();
    collectibles.clear();

    // Custom data
    auto jsonText = loadTextFile(levelFile);
    auto json = nlohmann::json::parse(jsonText);
    auto basePath = std::filesystem::path(levelFile).parent_path();

    levelDescription = loadUnicodeStringFromJson(json, "description");
    tileSize = json["tileSize"].get<int>();
    extraLevelEndDelay = json["extraLevelEndDelay"].get<float>();

    auto musicPath = json["music"].get<std::string>();
    auto musicVolume = json["musicVolume"].get<float>();
    music.Load((basePath / musicPath).string());
    music.SetVolume(musicVolume);

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
        paralaxHaxxorOffsets.push_back(layer["paralaxHaxxorOffset"].get<float>());

        paralaxLayers.emplace_back((basePath / imagePath).string());
        paralaxLayers.back().SetWrap(TEXTURE_WRAP_REPEAT); // For this to work textures must have power of 2 dimensions.
        paralaxScales.emplace_back(scaleX, scaleY);
    }

    // LDtk data
    auto ldtkDir = basePath / json["ldtkMap"].get<std::string>();
    auto ldtkDataText = loadTextFile((ldtkDir / "data.json").string());
    auto ldtkData = nlohmann::json::parse(ldtkDataText);

    levelWidth = ldtkData["width"].get<int>();
    levelHeight = ldtkData["height"].get<int>();
    ZASSERT(levelWidth % tileSize == 0);
    ZASSERT(levelHeight % tileSize == 0);

    playerStartPosition = loadJsonRect(ldtkData["entities"]["PlayerStart"][0]).GetPosition();
    levelExit = loadJsonRect(ldtkData["entities"]["Exit"][0]);
    levelExitDoor = loadJsonRect(ldtkData["entities"]["ExitDoor"][0]);
    furharkBubble = loadJsonRect(ldtkData["entities"]["Futhark"][0]);
    furharkTrigger = loadJsonRect(ldtkData["entities"]["FurharkTrigger"][0]);

    for (const auto& collectible : ldtkData["entities"]["Collectible"]) {
        collectibles.emplace_back(game.collectiblePrefab);
        collectibles.back().position = loadJsonRect(collectible).GetPosition();
    }

    // IntGrid
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
    music.Seek(0);
    music.Play();
    game.player.setInitialState(playerStartPosition);

    showFuthark = false;
    showFutharkStartTime = 0.0f;

    levelEnding = false;
    levelEndingStartTime = 0.0f;
    levelEndingByDeath = false;
}

void Level::endLevel() {
    music.Stop();
}

void Level::drawBackground() {
    for (int i = 0; i < std::ssize(paralaxLayers); ++i) {
        auto pos = game.cameraPosition * paralaxScales[i];
        pos.y += paralaxHaxxorOffsets[i];
        //auto pos = raylib::Vector2::Zero();
        //paralaxLayers[i].Draw(game.worldToScreen(pos));
        //game.drawSprite(pos, paralaxLayers[i], raylib::Vector2::Zero(), false);
        //paralaxLayers[i].Draw(raylib::Rectangle {pos.x, pos.y, game.screenWidth * 1.0f, game.screenHeight * 1.0f}, raylib::Rectangle {0, 0, game.screenWidth * 1.0f, game.screenHeight * 1.0f});
        DrawTextureTiled(paralaxLayers[i], raylib::Rectangle {pos.x, pos.y, game.screenWidth * 1.0f, game.screenHeight * 1.0f}, raylib::Rectangle {0, 0, game.screenWidth * 1.0f, game.screenHeight * 1.0f});
    }

    for (int i = 0; i < std::ssize(backgrounds); ++i) {
        backgrounds[i].Draw(game.worldToScreen({ 0.0f, 0.0f }));
    }

    if (levelEnding && !levelEndingByDeath) {
        auto animTime = game.levelTime - levelEndingStartTime;
        if (animTime > exitDoorAnimation.getAnimationLength() / 2) {
            // Hack
            game.player.playerHide = true;
        }
        auto [origin, image, sound] = exitDoorAnimation.spriteForTime(animTime);
        if (sound)
            sound->Play();
        game.drawSprite(levelExitDoor.GetPosition(), image, origin, false);
    }

    if (showFuthark) {
        auto animTime = game.levelTime - showFutharkStartTime;
        if (animTime >= futharkAnimation.getAnimationLength()) {
            animTime = futharkAnimation.getAnimationLength() - 0.01f;
        }
        auto [origin, image, sound] = futharkAnimation.spriteForTime(animTime);
        if (sound)
            sound->Play();
        game.drawSprite(furharkBubble.GetPosition(), image, origin, false);
    }
}

void Level::update() {
    for (int i = 0; i < std::ssize(foregrounds); ++i) {
        foregrounds[i].Draw(game.worldToScreen({ 0.0f, 0.0f }));
    }

    for (auto& collectible : collectibles) {
        collectible.update();
    }
}

std::optional<TileType> Level::getTileRaw(int x, int y) const {
    if (x < 0) return {};
    if (y < 0) return {};
    if (x * tileSize >= levelWidth) return {};
    if (y * tileSize >= levelHeight) return {};

    auto index = y * levelWidth / tileSize + x;
    return static_cast<TileType>(levelData[index]);
}

std::optional<TileType> Level::getTileWorld(raylib::Vector2 worldPosition) const {
    if ((worldPosition.x < 0) || (worldPosition.y < 0))
        return {};
    return getTileRaw(static_cast<int>(worldPosition.x) / tileSize, static_cast<int>(worldPosition.y) / tileSize);
}

/// Performs collision detection and response.
/// returns timeOfImpact
float Level::collisionDetection(raylib::Rectangle hitBox, raylib::Vector2 velocity) {
    ZASSERT(hitBox.GetWidth() < tileSize);
    ZASSERT(hitBox.GetHeight() < tileSize);

    // To avoid tunnelling through cells we do steps at most tileSize long.
    auto speed = velocity.Length();
    auto stepSpeed = tileSize * 0.25f; // If we jump a full tile we overlap, but why???
    auto [result, remainder] = divide(speed, stepSpeed);
    auto numSteps = result + 1;

    auto stepVelocity = velocity * (stepSpeed / speed);
    for (int step = 0; step < numSteps; ++step) {
        if (step == numSteps - 1) {
            stepVelocity = velocity * (remainder / speed);
        }

        auto hitBoxTileX = static_cast<int>(hitBox.GetPosition().x) / tileSize;
        auto hitBoxTileY = static_cast<int>(hitBox.GetPosition().y) / tileSize;
        if (hitBox.GetPosition().x < 0)
            hitBoxTileX -= 1;
        if (hitBox.GetPosition().y < 0)
            hitBoxTileY -= 1;

        /// Return time of impact when colliding with given tile, using given velocity.
        /// @param x    Relative tile position, -1..1.
        /// @param y    Relative tile position, -1..1.
        /// @return Time of impact: 0.0..1.0
        auto boxToi = [this, hitBox, hitBoxTileX, hitBoxTileY](int x, int y, raylib::Vector2 velocity) -> float {
            auto tile = getTileRaw(hitBoxTileX + x, hitBoxTileY + y).value_or(TileType::WALL);
            if (!isCollider(tile))
                return 1.0f;

            raylib::Rectangle blocker = {
                (hitBoxTileX + x) * tileSize * 1.0f,
                (hitBoxTileY + y) * tileSize * 1.0f,
                tileSize * 1.0f,
                tileSize * 1.0f,
            };
            game.drawScreenRect(hitBox, YELLOW, 20);
            auto [collision, toi] = game.collideBoxes(hitBox, velocity, blocker);
            if (toi == 1.0f)
                game.drawScreenRect(blocker, SKYBLUE, 1);
            else
                game.drawScreenRect(blocker, WHITE, 10);
            return toi;
        };

        auto timeOfImpact = 1.0f;
        for (int i = -1; i <= 1; ++i)
            for (int j = -1; j <= 1; ++j) {
                if ((i == 0) && (j == 0))
                    continue;

                // We don't want to collide with
                bool ignoreCollider = false;

                auto toi = boxToi(j, i, stepVelocity);
                timeOfImpact = std::min(timeOfImpact, toi);
            }

        if (timeOfImpact < 1.0f)
            return (step * 1.0f + timeOfImpact) / numSteps;

        hitBox.SetPosition(hitBox.GetPosition() + stepVelocity);
    }

    return 1.0f;
}

/// Performs collision detection and response.
/// @note assumes hitBoxes are smaller than a tile.
/// @note Implementation is weak, and also assumes that colliders don't touch with just corners.
/// returns (grounded, touchingCeiling, touchingWall, touchingWallDirection)
std::tuple<bool, bool, bool, int> Level::collisionQuery(raylib::Rectangle hitBox) {
    ZASSERT(hitBox.GetWidth() < tileSize);
    ZASSERT(hitBox.GetHeight() < tileSize);

    auto hitBoxTileX = static_cast<int>(hitBox.GetPosition().x) / tileSize;
    auto hitBoxTileY = static_cast<int>(hitBox.GetPosition().y) / tileSize;
    if (hitBox.GetPosition().x < 0)
        hitBoxTileX -= 1;
    if (hitBox.GetPosition().y < 0)
        hitBoxTileY -= 1;

    /// Return time of impact when colliding with given tile, using given velocity.
    /// @param x    Relative tile position, -1..1.
    /// @param y    Relative tile position, -1..1.
    /// @return Time of impact: 0.0..1.0
    auto boxToi = [this, hitBox, hitBoxTileX, hitBoxTileY](int x, int y, raylib::Vector2 velocity) -> float {
        auto tile = getTileRaw(hitBoxTileX + x, hitBoxTileY + y).value_or(TileType::WALL);
        if (!isCollider(tile))
            return 1.0f;

        raylib::Rectangle blocker = {
            (hitBoxTileX + x) * tileSize * 1.0f,
            (hitBoxTileY + y) * tileSize * 1.0f,
            tileSize * 1.0f,
            tileSize * 1.0f,
        };
        auto [collision, toi] = game.collideBoxes(hitBox, velocity, blocker);
        return toi;
    };

    auto grounded = false, touchingCeiling = false, touchingWall = false;
    auto touchingWallDirection = -1;

    const float epsilon = 0.01;

    // Floor
    for (int j = -1; j <= 1; ++j) {
        auto toi = boxToi(j, 1, raylib::Vector2(0.0f, 1.0f));
        if (toi < epsilon) {
            grounded = true;
        }
    }

    // Ceiling
    for (int j = -1; j <= 1; ++j) {
        auto toi = boxToi(j, -1, raylib::Vector2(0.0f, -1.0f));
        if (toi < epsilon) {
            touchingCeiling = true;
        }
    }

    // Left wall
    for (int j = -1; j <= 1; ++j) {
        auto toi = boxToi(-1, j, raylib::Vector2(-1.0f, 0.0f));
        if (toi < epsilon) {
            touchingWall = true;
            touchingWallDirection = -1;
        }
    }

    // Right wall
    for (int j = -1; j <= 1; ++j) {
        auto toi = boxToi(1, j, raylib::Vector2(1.0f, 0.0f));
        if (toi < epsilon) {
            touchingWall = true;
            touchingWallDirection = 1;
        }
    }

    return { grounded, touchingCeiling, touchingWall, touchingWallDirection };
}

std::tuple<int, int> Level::getCollectibleStats() const {
    int collectedCount = 0;
    int totalCount = 0;
    for (auto& collectible : collectibles) {
        if (collectible.collected) collectedCount++;
        if (!collectible.forHud) totalCount++;
    }
    return { collectedCount, totalCount };
}

void Level::setShowFuthark() {
    if (showFuthark) return;
    showFuthark = true;
    showFutharkStartTime = game.levelTime;
}

void Level::setLevelEnding(bool death) {
    if (levelEnding) return;
    levelEnding = true;
    levelEndingStartTime = game.levelTime;
    levelEndingByDeath = death;
}

bool Level::hasLevelEnded() const {
    if (!levelEnding) return false;
    auto animTime = game.levelTime - levelEndingStartTime;
    if (animTime > exitDoorAnimation.getAnimationLength() + extraLevelEndDelay)
        return true;
    return false;
}
