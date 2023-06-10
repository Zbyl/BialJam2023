
#include "Scene.h"

#include "Game.h"

#include "Utilities.h"

#include "zerrors.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <numeric>


void Scene::startScene() {
    game.menu.setMenuRectangle(menuRectangle);
    animTime = 0.0f;
    music.Seek(0);
    music.Play();
}

void Scene::endScene() {
    music.Stop();
}

void Scene::load(const std::string& sceneFile) {
    auto jsonText = loadTextFile(sceneFile);

    auto json = nlohmann::json::parse(jsonText);

    auto basePath = std::filesystem::path(sceneFile).parent_path();

    auto musicPath = json["music"].get<std::string>();
    auto musicVolume = json["musicVolume"].get<float>();
    music.Load((basePath / musicPath).string());
    music.SetVolume(musicVolume);

    sceneDelay = json["sceneDelay"].get<float>();
    menuRectangle = loadJsonRect(json["menuRectangle"]);

    for (auto animation : json["animations"]) {
        auto positionX = animation["position"]["x"].get<float>();
        auto positionY = animation["position"]["y"].get<float>();
        positions.emplace_back(positionX, positionY);
        if (animation.contains("delay")) {
            delays.push_back(animation["delay"].get<float>());
        } else {
            delays.push_back(0.0f);
        }
        if (animation.contains("image")) {
            auto imagePath = animation["image"].get<std::string>();
            animations.emplace_back();
            animations.back().fromPicture((basePath / imagePath).string());
        }
        else {
            auto animationPath = animation["animation"].get<std::string>();
            animations.emplace_back();
            animations.back().load((basePath / animationPath).string());
        }
    }
}

void Scene::update() {
    music.Update();

    animTime += game.window.GetFrameTime();

    for (int i = 0; i < std::ssize(animations); ++i) {
        auto delay = delays[i];
        if (animTime < delay)
            continue;
        auto [origin, image, sound] = animations[i].spriteForTime(animTime - delay);
        image.Draw(positions[i] - origin);
        if (sound)
            sound->Play();
    }
}

bool Scene::areAnimationsFinished() const {
    if (animTime < sceneDelay) {
        return false;
    }

    for (int i = 0; i < std::ssize(animations); ++i) {
        if (animTime < animations[i].getAnimationLength())
            return false;
    }

    return true;
}