#include "raylib-cpp.hpp"

#include "Game.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


int main()
{
    Game game;

    game.mainLoop();

    return 0;
}
