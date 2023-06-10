
#include "Collectible.h"

#include "Game.h"
#include "Utilities.h"

#include "zstr.h"

#include "nlohmann/json.hpp"

#include <cmath>

void CollectiblePrefab::load() {
    const std::string playerFile = "Graphics/Collectible/collectible.json";
    auto jsonText = loadTextFile(playerFile);
    auto json = nlohmann::json::parse(jsonText);
    hitbox = raylib::Rectangle{ json["hitbox"]["x"].get<float>(), json["hitbox"]["y"].get<float>(), json["hitbox"]["width"].get<float>(), json["hitbox"]["height"].get<float>() };
}

void Collectible::update() {
    if (collected) {
        return;
    }

    raylib::Rectangle collectibleRect { position + collectiblePrefab.hitbox.GetPosition(), collectiblePrefab.hitbox.GetSize() };
    raylib::Rectangle playerRect { collectiblePrefab.game.player.position + collectiblePrefab.game.player.hitbox.GetPosition(), collectiblePrefab.game.player.hitbox.GetSize() };
    if (!forHud && playerRect.CheckCollision(collectibleRect)) {
        collected = true;
        collectiblePrefab.collectSfx.Play();
        return;
    }

    animTime += collectiblePrefab.game.levelTimeDelta;

    auto [origin, image, sound] = collectiblePrefab.wiggleAnimation.spriteForTime(animTime);
    if (sound)
        sound->Play();
    if (forHud)
        image.Draw(position - origin);
    else
        collectiblePrefab.game.drawSprite(position, image, origin, false);
}
