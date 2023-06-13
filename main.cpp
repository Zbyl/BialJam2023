#include "raylib-cpp.hpp"

#include "Game.h"

#include "zerrors.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

std::unique_ptr<Game> global_game;

void updateDrawFrame() {
    if (!global_game) {
        global_game.reset(new Game()); // We must initialize window after emscripten main loop is defined.
        global_game->restartGame();
    }
    global_game->drawFrame();
}

int main()
{
    try
    {
        SetConfigFlags(FLAG_VSYNC_HINT);

#if defined(PLATFORM_WEB)
        emscripten_set_main_loop(updateDrawFrame, 0, 1);
#else
        SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
        global_game.reset(new Game());
        global_game->restartGame();
        global_game->mainLoop();
#endif
    }
    catch (const std::exception& exc) {
        TraceLog(LOG_ERROR, "Exception:");
        TraceLog(LOG_ERROR, exc.what());
        return 1;
    }

    return 0;
}
