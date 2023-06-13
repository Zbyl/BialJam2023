
#include "ResourceCache.h"


raylib::Texture2D* ResourceCache::getImage(const std::string& fileName) {
    auto it = imageCache.find(fileName);
    if (it != imageCache.end())
        return it->second.get();

    auto [newit, inserted] = imageCache.try_emplace(fileName, std::make_unique<raylib::Texture2D>(fileName));
    return newit->second.get();
}

raylib::Sound* ResourceCache::getSound(const std::string& fileName) {
    auto it = soundCache.find(fileName);
    if (it != soundCache.end())
        return it->second.get();

    auto [newit, inserted] = soundCache.try_emplace(fileName, std::make_unique<raylib::Sound>(fileName));
    return newit->second.get();
}
