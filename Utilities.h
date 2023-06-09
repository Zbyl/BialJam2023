#pragma once

#include "raylib-cpp.hpp"

#include "zerrors.h"

#include <string>

/// Exception thrown when a file is missing.
class FileNotFoundException : public terminal_editor::GenericException {};


std::string loadTextFile(const std::string& fileName);
