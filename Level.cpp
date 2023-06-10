
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
    backgrounds.clear();
    foregrounds.clear();
    paralaxLayers.clear();
    paralaxScales.clear();
    levelData.clear();
    collectibles.clear();

    // Custom data
    auto jsonText = loadTextFile(levelFile);
    auto json = nlohmann::json::parse(jsonText);
    auto basePath = std::filesystem::path(levelFile).parent_path();

    tileSize = json["tileSize"].get<int>();

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
        //paralaxLayers.back().SetWrap(TEXTURE_WRAP_REPEAT);
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
}

void Level::endLevel() {
    music.Stop();
}

void Level::drawBackground() {
    for (int i = 0; i < std::ssize(paralaxLayers); ++i) {
        auto pos = game.cameraPosition * paralaxScales[i];
        pos.y += paralaxHaxxorOffsets[i];
        //auto pos = raylib::Vector2::Zero();
        paralaxLayers[i].Draw(game.worldToScreen(pos));
        //game.drawSprite(pos, paralaxLayers[i], raylib::Vector2::Zero(), false);
        //paralaxLayers[i].Draw(raylib::Rectangle {pos.x, pos.y, game.screenWidth * 1.0f, game.screenHeight * 1.0f}, raylib::Rectangle {0, 0, game.screenWidth * 1.0f, game.screenHeight * 1.0f});
        DrawTextureTiled(paralaxLayers[i], raylib::Rectangle {pos.x, pos.y, game.screenWidth * 1.0f, game.screenHeight * 1.0f}, raylib::Rectangle {0, 0, game.screenWidth * 1.0f, game.screenHeight * 1.0f});
    }

    for (int i = 0; i < std::ssize(backgrounds); ++i) {
        backgrounds[i].Draw(game.worldToScreen({ 0.0f, 0.0f }));
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
/// @note assumes hitBoxes are smaller than a tile.
/// @note Implementation is weak, and also assumes that colliders don't touch with just corners.
/// returns (grounded, touchingCeiling, touchingWall, touchingWallDirection, moveDelta)
std::tuple<bool, bool, bool, int, raylib::Vector2> Level::collisionDetection(raylib::Rectangle hitBox, raylib::Vector2 velocity) {
    ZASSERT(hitBox.GetWidth() < tileSize);
    ZASSERT(hitBox.GetHeight() < tileSize);

    auto hitBoxTileX = static_cast<int>(hitBox.GetPosition().x) / tileSize;
    auto hitBoxTileY = static_cast<int>(hitBox.GetPosition().y) / tileSize;
    if (hitBox.GetPosition().x < 0)
        hitBoxTileX -= 1;
    if (hitBox.GetPosition().y < 0)
        hitBoxTileY -= 1;

    TileType blocks[4] = {
        getTileRaw(hitBoxTileX + 0, hitBoxTileY + 0).value_or(TileType::WALL),
        getTileRaw(hitBoxTileX + 1, hitBoxTileY + 0).value_or(TileType::WALL),
        getTileRaw(hitBoxTileX + 0, hitBoxTileY + 1).value_or(TileType::WALL),
        getTileRaw(hitBoxTileX + 1, hitBoxTileY + 1).value_or(TileType::WALL),
    };

    auto numColliders = 0;
    for (auto tile : blocks) {
        if (isCollider(tile)) numColliders++;
    }
    if (numColliders == 0)
        return { false, false, false, -1, raylib::Vector2::Zero() };

    auto topLeft = isCollider(blocks[0]);
    auto topRight = isCollider(blocks[1]);
    auto bottomLeft = isCollider(blocks[2]);
    auto bottomRight = isCollider(blocks[3]);

    auto topLeftFix = topLeft;
    auto topRightFix = topRight;
    auto bottomLeftFix = bottomLeft;
    auto bottomRightFix = bottomRight;

    auto hitBoxIsRight = (static_cast<int>(hitBox.GetX()) % tileSize) + hitBox.GetWidth() > tileSize;
    auto hitBoxIsDown = (static_cast<int>(hitBox.GetY()) % tileSize) + hitBox.GetHeight() > tileSize;

    enum class Direction {
        UP = 0,
        DOWN = 1,
        LEFT = 2,
        RIGHT = 3,
    };

    std::vector<Direction> moves;

    if (topLeft && topRight) {
        moves.push_back(Direction::DOWN);
        topLeftFix = false;
        topRightFix = false;
    }

    if (bottomLeft && bottomRight) {
        if (hitBoxIsDown)
            moves.push_back(Direction::UP);
        bottomLeftFix = false;
        bottomRightFix = false;
    }

    if (topLeft && bottomLeft) {
        moves.push_back(Direction::RIGHT);
        topLeftFix = false;
        bottomLeftFix = false;
    }

    if (topRight && bottomRight) {
        if (hitBoxIsRight)
            moves.push_back(Direction::LEFT);
        topRightFix = false;
        bottomRightFix = false;
    }

    if (topLeftFix) {
        if (velocity.x >= 0)
            moves.push_back(Direction::DOWN);
        else
            moves.push_back(Direction::RIGHT);
    }

    if (topRightFix) {
        if (velocity.x <= 0)
            moves.push_back(Direction::DOWN);
        else
            if (hitBoxIsRight)
                moves.push_back(Direction::LEFT);
    }

    if (bottomLeftFix) {
        if (velocity.x >= 0)
            if (hitBoxIsDown)
                moves.push_back(Direction::UP);
            else
                moves.push_back(Direction::RIGHT);
    }

    if (bottomRightFix) {
        if (velocity.x <= 0)
            if (hitBoxIsDown)
                moves.push_back(Direction::UP);
            else
                if (hitBoxIsRight)
                    moves.push_back(Direction::LEFT);
    }

    bool grounded = false;
    bool touchingWall = false;
    int touchingWallDirection = 0; // 1 right, -1 left, 0 not touching. If both sides, player direction is used.
    bool touchingCeiling = false;
    raylib::Vector2 moveDelta { 0.0f, 0.0f };

    for (auto direction : moves) {
        if (direction == Direction::DOWN) {
            touchingCeiling = true;
            moveDelta.y = tileSize - std::get<1>(divide(hitBox.GetY(), tileSize));
            continue;
        }
        if (direction == Direction::UP) {
            grounded = true;
            moveDelta.y = -std::get<1>(divide(hitBox.GetY() + hitBox.GetHeight(), tileSize));
            continue;
        }
        if (direction == Direction::RIGHT) {
            touchingWall = true;
            touchingWallDirection = -1;
            moveDelta.x = tileSize - std::get<1>(divide(hitBox.GetX(), tileSize));
            continue;
        }
        if (direction == Direction::LEFT) {
            touchingWall = true;
            touchingWallDirection = 1;
            moveDelta.x = -std::get<1>(divide(hitBox.GetX(), tileSize));
            continue;
        }
    }

    auto groundLevel = hitBoxTileY * tileSize;
    auto groundCenter = (hitBoxTileX + 1) * tileSize;
    auto movedHitBoxBottom = (hitBox.GetPosition() + hitBox.GetSize() + moveDelta).y;
    auto movedHitBoxLeft = (hitBox.GetPosition() + moveDelta).x;
    auto movedHitBoxRight = (hitBox.GetPosition() + hitBox.GetSize() + moveDelta).x;
    if (movedHitBoxBottom + 1 >= groundLevel) {
        if (bottomLeft && (movedHitBoxLeft < groundCenter)) {
            grounded = true;
        }
        if (bottomRight && (movedHitBoxRight >= groundCenter)) {
            grounded = true;
        }
    }

    return { grounded, touchingCeiling, touchingWall, touchingWallDirection, moveDelta };
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
