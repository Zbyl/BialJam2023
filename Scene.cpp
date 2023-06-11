
#include "Scene.h"

#include "Game.h"

#include "Utilities.h"

#include "zerrors.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <numeric>


void Scene::startScene(bool forReload) {
    game.menu.setMenuRectangle(menuRectangle);
    animTime = 0.0f;
    if (!forReload) {
        music.Seek(0);
        music.Play();
    }
}

void Scene::endScene() {
    music.Stop();
}

void Scene::load(const std::string& sceneFile, bool useFuthark, bool reloadHack) {
    auto jsonText = loadTextFile(sceneFile);

    auto json = nlohmann::json::parse(jsonText);

    auto basePath = std::filesystem::path(sceneFile).parent_path();

    if (reloadHack) // Make loading faster!
    {
        auto imagePath = json["animations"][0]["image"].get<std::string>();
        auto imPath = (basePath / imagePath).string();
        if (useFuthark) {
            size_t start_pos = imPath.find(".png");
            imPath.replace(start_pos, 0, "-vr");
        }
        animations[0].fromPicture(imPath);
        return;
    }

    animations.clear();
    positions.clear();
    delays.clear();


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
        auto loop = true;
        if (animation.contains("loop")) {
            loop = animation["loop"].get<bool>();
        }
        if (animation.contains("image")) {
            auto imagePath = animation["image"].get<std::string>();
            animations.emplace_back();
            auto imPath = (basePath / imagePath).string();
            if (useFuthark) {
                size_t start_pos = imPath.find(".png");
                imPath.replace(start_pos, 0, "-vr");
            }
            animations.back().fromPicture(imPath);
        }
        else {
            auto animationPath = animation["animation"].get<std::string>();
            animations.emplace_back();
            animations.back().load((basePath / animationPath).string());
            animations.back().loop = loop;
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