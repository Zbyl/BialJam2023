#include "game.h"

raylib::Vector2 Game::worldToScreen(raylib::Vector2 worldPosition) {
    auto delta = worldPosition - cameraPosition;
    return { delta.x + screenWidth / 2, -delta.y + screenHeight / 2 };
}

raylib::Vector2 Game::screenToWorld(raylib::Vector2 screenPosition) {
    return { screenPosition.x - screenWidth / 2 + cameraPosition.x, -(screenPosition.y - screenHeight / 2) + cameraPosition.y };
}


void Game::mainLoop()
{
    raylib::Texture2D texXboxPad("Menus/xbox.png");

    raylib::Color textColor = raylib::Color::LightGray();

    raylib::Vector2 pos(190, 200);

    raylib::Music music(R"(Music\platformer_level03.mp3)");
    music.Play();

    // Main game loop, detect window close button, ESC key or programmatic quit.
    while (!window.ShouldClose() && !shouldQuit)
    {
        menu.update();
        if (!menu.isInMenu())
        {
            levelTimeDelta = window.GetFrameTime();
            levelTime += window.GetFrameTime();
            player.update();
        }
        else {
            levelTimeDelta = 0;
        }

        music.Update();

        raylib::Gamepad gamepad(0);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        {
            window.ClearBackground(RAYWHITE);

            if (gamepad.IsAvailable())
            {
                pos.x += gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_X);
                pos.y += gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y);

                DrawText(TextFormat("GP1: %s", gamepad.GetName().c_str()), 10, 10, 10, BLACK);

                texXboxPad.Draw(0, 0, DARKGRAY);

                // Draw buttons: xbox home
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_MIDDLE))
                    DrawCircle(394, 89, 19, RED);

                // Draw buttons: basic
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_MIDDLE_RIGHT))
                    DrawCircle(436, 150, 9, RED);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_MIDDLE_LEFT))
                    DrawCircle(352, 150, 9, RED);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) // X
                    DrawCircle(501, 151, 15, BLUE);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) // A
                    DrawCircle(536, 187, 15, LIME);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) // B
                    DrawCircle(572, 151, 15, MAROON);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_UP)) // Y
                    DrawCircle(536, 115, 15, GOLD);

                // Draw buttons: d-pad
                DrawRectangle(317, 202, 19, 71, BLACK);
                DrawRectangle(293, 228, 69, 19, BLACK);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_UP))
                    DrawRectangle(317, 202, 19, 26, RED);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_DOWN))
                    DrawRectangle(317, 202 + 45, 19, 26, RED);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_LEFT))
                    DrawRectangle(292, 228, 25, 19, RED);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
                    DrawRectangle(292 + 44, 228, 26, 19, RED);

                // Draw buttons: left-right back
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_TRIGGER_1))
                    DrawCircle(259, 61, 20, RED);
                if (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
                    DrawCircle(536, 61, 20, RED);

                // Draw axis: left joystick
                DrawCircle(259, 152, 39, BLACK);
                DrawCircle(259, 152, 34, LIGHTGRAY);
                DrawCircle(259 + (int)(gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_X) * 20), 152 + (int)(gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y) * 20), 25,
                    gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_THUMB) ? RED : BLACK);

                // Draw axis: right joystick
                DrawCircle(461, 237, 38, BLACK);
                DrawCircle(461, 237, 33, LIGHTGRAY);
                DrawCircle(461 + (int)(gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_X) * 20), 237 + (int)(gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_Y) * 20), 25,
                    gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_THUMB) ? RED : BLACK);

                // Draw axis: left-right triggers
                DrawRectangle(170, 30, 15, 70, GRAY);
                DrawRectangle(604, 30, 15, 70, GRAY);
                DrawRectangle(170, 30, 15, (int)(((1 + gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_TRIGGER)) / 2) * 70), RED);
                DrawRectangle(604, 30, 15, (int)(((1 + gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_TRIGGER)) / 2) * 70), RED);
            }

            DrawText(TextFormat("DETECTED AXIS [%i]:", gamepad.GetAxisCount()), 10, 50, 10, MAROON);

            for (int i = 0; i < gamepad.GetAxisCount(); i++)
            {
                DrawText(TextFormat("AXIS %i: %.02f", i, gamepad.GetAxisMovement(i)), 20, 70 + 20 * i, 10, DARKGRAY);
            }

            if (GetGamepadButtonPressed() != -1)
                DrawText(TextFormat("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 430, 10, RED);
            else
                DrawText("DETECTED BUTTON: NONE", 10, 430, 10, GRAY);

            textColor.DrawText("Congrats! You created your first window!", pos.x, pos.y, 20);
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
}

