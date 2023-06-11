
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

    animTime += collectiblePrefab.game.levelTimeDelta;

    auto [origin, image, sound] = collectiblePrefab.wiggleAnimation.spriteForTime(animTime);

    raylib::Rectangle collectibleRect { position - origin + collectiblePrefab.hitbox.GetPosition(), collectiblePrefab.hitbox.GetSize() };

    //auto hitBoxPosition = collectiblePrefab.game.worldToScreen(collectibleRect.GetPosition());
    //DrawRectangleLines(hitBoxPosition.x, hitBoxPosition.y, collectibleRect.GetWidth(), collectibleRect.GetHeight(), GREEN);

    //auto [playerOrigin, playerImage, playerSound] = collectiblePrefab.game.player.runAnimation.spriteForTime(0.0f);
    //raylib::Rectangle playerRect { collectiblePrefab.game.player.position - playerOrigin + collectiblePrefab.game.player.hitbox.GetPosition(), collectiblePrefab.game.player.hitbox.GetSize() };
    if (!forHud && collectibleRect.CheckCollision(collectiblePrefab.game.player.position)) {
        collected = true;
        collectiblePrefab.collectSfx.Play();
        return;
    }

    if (sound)
        sound->Play();
    if (forHud)
        image.Draw(position - origin);
    else
        collectiblePrefab.game.drawSprite(position, image, origin, false);
}
