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
