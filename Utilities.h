#pragma once

#include "raylib-cpp.hpp"

#include "zerrors.h"

#include <string>
#include <tuple>

#include "nlohmann/json.hpp"


/// Exception thrown when a file is missing.
class FileNotFoundException : public terminal_editor::GenericException {};


std::string loadTextFile(const std::string& fileName);

/// Divides value by divisor.
/// @param value    Value to divide.
/// @param divisor  Must be > 0.
/// @returns (result, remainder) - how many times divisor fits in value, and a remainder.
///          So: result * divisor + remainder = value.
///          remainder is always non-negative.
std::tuple<int, float> divide(float value, float divisor);

raylib::Rectangle loadJsonRect(const nlohmann::json& json);

void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin = {0.0f, 0.0f}, float rotation = 0.0f, float scale = 1.0f, Color tint = WHITE);

/// Converts u32 string into something that can be displayed using given font.
/// @param allowLowercase   If false lower case characters are replaced with uppercase.
/// @param allowDiacritics  If false diacritics are replaced with non-diacritic versions of characters.
std::string textForFont(bool allowLowercase, bool allowDiacritics, const std::u32string& text);

/// Loads codepoints from a file.
/// Throws if there are any duplicates.
std::vector<int> loadCharset(const std::string& fileName);

/// Loads UTF-8 string from JSON and returns it as UTF-32 string.
std::u32string loadUnicodeStringFromJson(const nlohmann::json& json, const std::string& key);
