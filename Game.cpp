#include "game.h"

#include "zstr.h"


raylib::Vector2 Game::worldToScreen(raylib::Vector2 worldPosition) const {
    auto delta = worldPosition - cameraPosition;
    return { delta.x + screenWidth / 2, delta.y + screenHeight / 2 };
}

raylib::Vector2 Game::screenToWorld(raylib::Vector2 screenPosition) const {
    return { screenPosition.x - screenWidth / 2 + cameraPosition.x, (screenPosition.y - screenHeight / 2) + cameraPosition.y };
}

void Game::drawHud(bool withTotals) {
    if (!withTotals) {
        auto startX = 1200.0f;
        auto startY = 30.0f;
        auto [collectedCount, totalCount] = level.getCollectibleStats();
        hudCollectible.position = raylib::Vector2{ startX, startY };
        hudCollectible.update();
        hudFont.DrawText((ZSTR() << collectedCount).str(), raylib::Vector2{ startX + 30.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
    } else {
        auto startX = 100.0f;
        auto startY = 250.0f;
        auto [collectedCount, totalCount] = level.getCollectibleStats();
        hudCollectible.position = raylib::Vector2{ startX, startY };
        hudCollectible.update();
        hudFont.DrawText((ZSTR() << collectedCount << " / " << totalCount << " w poziomie").str(), raylib::Vector2{ startX + 30.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);

        startX = 930.0f;
        startY = 250.0f;
        hudCollectible.position = raylib::Vector2{ startX, startY };
        hudCollectible.update();
        hudFont.DrawText((ZSTR() << totalCollected << " / " << totalAvailable << " w sumie").str(), raylib::Vector2{ startX + 30.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
    }
}

void Game::cameraUpdate() {
    raylib::Rectangle wndRect = { player.cameraWindow.x * screenWidth, player.cameraWindow.y * screenHeight, player.cameraWindow.width * screenWidth, player.cameraWindow.height * screenHeight };
    raylib::Vector2 screenHalfSize { screenWidth / 2.0f, screenHeight / 2.0f };
    raylib::Rectangle cameraWindow(cameraPosition - screenHalfSize + wndRect.GetPosition(), wndRect.GetSize()); // In world coordinates.

    if (!cameraWindow.CheckCollision(player.position)) {
        // Move camera to put player inside.
        if (player.position.x < cameraWindow.x) cameraWindow.x = player.position.x;
        if (player.position.x > cameraWindow.x + cameraWindow.width) cameraWindow.x = player.position.x - cameraWindow.width;
        if (player.position.y < cameraWindow.y) cameraWindow.y = player.position.y;
        if (player.position.y > cameraWindow.y + cameraWindow.height) cameraWindow.y = player.position.y - cameraWindow.height;
    }

    cameraPosition = cameraWindow.GetPosition() + cameraWindow.GetSize() / 2.0f;

    // Move camera to be within level bounds.
    raylib::Rectangle cameraView(cameraPosition - screenHalfSize, screenHalfSize * 2); // In world coordinates.
    if (cameraView.x < 0.0f) cameraWindow.x -= cameraView.x;
    if (cameraView.y < 0.0f) cameraWindow.y -= cameraView.y;
    if (cameraView.x + cameraView.width > level.levelWidth) cameraWindow.x -= (cameraView.x + cameraView.width) - level.levelWidth;
    if (cameraView.y + cameraView.height > level.levelHeight) cameraWindow.y -= (cameraView.y + cameraView.height) - level.levelHeight;

    cameraPosition = cameraWindow.GetPosition() + cameraWindow.GetSize() / 2.0f;
}

void Game::restartGame() {
    level.endLevel();
    for (auto screen : { &startScreen, &deadScreen, &levelEndScreen, &gameEndScreen })
        screen->endScene();

    menu.setInMenu(true);
    gameState = GameState::START_SCREEN;
    totalCollected = 0;
    startScreen.startScene();
    currentLevel = 0;
}

void Game::restartLevel() {
    startLevel(currentLevel);
}

void Game::startLevel(int levelIndex) {
    menu.setInMenu(false);
    menu.setMenuRectangle({ 0.0f, 200.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight) });
    gameState = GameState::LEVEL;
    currentLevel = levelIndex;
    ZASSERT(currentLevel < std::ssize(levelFiles));
    level.load(levelFiles[currentLevel]);
    level.startLevel();
    cameraPosition = player.position;
    cameraUpdate();
}

void Game::endLevel(bool died) {
    menu.setInMenu(false);
    level.endLevel();
    for (auto screen : { &startScreen, &deadScreen, &levelEndScreen, &gameEndScreen })
        screen->endScene();

    if (died) {
        gameState = GameState::LEVEL_DIED;
        deadScreen.startScene();
    }
    else
    {
        if (currentLevel < std::ssize(levelFiles) - 1) {
            gameState = GameState::LEVEL_SUCCESS;
            levelEndScreen.startScene();
        }
        else {
            gameState = GameState::GAME_SUCCESS;
            gameEndScreen.startScene();
        }

        auto [collectedCount, totalCount] = level.getCollectibleStats();
        totalCollected += collectedCount;
        totalAvailable += totalCount;
    }
}

void Game::mainLoop()
{
    bool debug = false;
    raylib::Vector2 hitboxVelocity = { 0, 0 };
    raylib::Rectangle hitbox = { 0, 0, 0, 0 };
    hitbox.SetPosition(player.position);

    restartGame();

    bool endLevelByDeath = false;

    // Main game loop, detect window close button, ESC key or programmatic quit.
    while (!window.ShouldClose() && !shouldQuit)
    {
        if (debug) {
            if (gamepad.IsButtonPressed(GAMEPAD_BUTTON_MIDDLE_LEFT)) {
                player.state = PlayerState::GROUNDED;
                player.position = level.playerStartPosition;
                player.velocity = raylib::Vector2::Zero();
                player.load();
            }
        }

        if (IsKeyPressed(KEY_SPACE))
            debug = !debug;

        if (debug) {
            if (IsKeyPressed(KEY_E))
                endLevel(false);
            if (IsKeyPressed(KEY_D))
                endLevel(true);
            if (IsKeyPressed(KEY_R))
                restartGame();
        }

        BeginDrawing();
        window.ClearBackground(RAYWHITE);

        menu.update();

        bool hackDisableMenuUpdate = false;

        if (gameState == GameState::LEVEL) {
            if (!menu.isInMenu())
            {
                levelTimeDelta = window.GetFrameTime();
                levelTime += window.GetFrameTime();

                player.update();
                cameraUpdate();

                level.drawBackground();
                player.draw();
                level.update();

                drawHud(false);

                if (level.levelExit.CheckCollision(player.position)) {
                    endLevelByDeath = false;
                    level.setLevelEnding(false);
                    player.setPlayerDead(false);
                }

                if (player.state == PlayerState::GROUNDED) {
                    auto playerTile = level.getTileWorld(player.position + raylib::Vector2(0, level.tileSize / 2)).value_or(TileType::EMPTY);
                    if (playerTile == TileType::LAVA) {
                        endLevelByDeath = true;
                        level.setLevelEnding(true);
                        player.setPlayerDead(true);
                    }
                }

                if (level.hasLevelEnded()) {
                    endLevel(endLevelByDeath);
                }
            }
            else {
                levelTimeDelta = 0;
            }

            level.music.Update(); // We want to update music even if level update was not called.
        }
        else
        if (gameState == GameState::START_SCREEN)
        {
            startScreen.update();
        }
        else
        if (gameState == GameState::LEVEL_SUCCESS)
        {
            levelEndScreen.update();
            drawHud(true);
            if (levelEndScreen.areAnimationsFinished()) {
                if (gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
                    startLevel(currentLevel + 1);
                    hackDisableMenuUpdate = true;
                }
            }
        }
        else
        if (gameState == GameState::LEVEL_DIED)
        {
            deadScreen.update();
            if (deadScreen.areAnimationsFinished()) {
                if (!menu.isInMenu() && gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
                    menu.setInMenu(true);
                    hackDisableMenuUpdate = true;
                }
            }
        }
        else
        if (gameState == GameState::GAME_SUCCESS)
        {
            gameEndScreen.update();
            drawHud(true);
            if (gameEndScreen.areAnimationsFinished()) {
                if (gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
                    restartGame();
                    hackDisableMenuUpdate = true;
                }
            }
        }

        if (!hackDisableMenuUpdate)
            menu.draw();

        if (debug) {
            // Debug Camera Window
            auto cameraScreenPosition = worldToScreen(cameraPosition);
            raylib::Rectangle wndRect = { player.cameraWindow.x * screenWidth, player.cameraWindow.y * screenHeight, player.cameraWindow.width * screenWidth, player.cameraWindow.height * screenHeight };
            DrawRectangleLines(wndRect.x, wndRect.y, wndRect.width, wndRect.height, BLUE);

            // Debug HitBox
            hitbox.SetSize(player.hitbox.GetSize());
            if (gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_THUMB)) {
                hitboxVelocity.x = gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_X) * (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ? 10.0f : 50.0f);
                hitboxVelocity.y = gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_Y) * (gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ? 10.0f : 50.0f);
            }
            else {
                hitbox.x += gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_X);
                hitbox.y += gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_Y);
            }

            auto [grounded, touchingCeiling, touchingWall, touchingWallDirection, moveDelta] = level.collisionDetection(hitbox, hitboxVelocity);
            auto hitBoxPosition = worldToScreen(hitbox.GetPosition());
            auto hitBoxCenter = worldToScreen(hitbox.GetPosition() + hitbox.GetSize() / 2);
            auto arrowPoint = hitBoxCenter + hitboxVelocity;
            DrawRectangleLines(hitBoxPosition.x, hitBoxPosition.y, hitbox.GetWidth(), hitbox.GetHeight(), GREEN);
            DrawLine(hitBoxCenter.x, hitBoxCenter.y, arrowPoint.x, arrowPoint.y, RED);
            if (grounded) DrawText("GROUNDED", 10, 300, 10, BLACK);
            if (touchingCeiling) DrawText("CEILING", 10, 310, 10, BLACK);
            if (touchingWall) DrawText((ZSTR() << "WALL ON " << ((touchingWallDirection == 1) ? "RIGHT" : "LEFT")).str().c_str(), 10, 320, 10, BLACK);
            DrawText((ZSTR() << "MOVE DELTA X: " << moveDelta.x << " Y: " << moveDelta.y).str().c_str(), 10, 330, 10, BLACK);

            DrawText((ZSTR() << "GAME STATE: " << to_string(gameState)).str().c_str(), 10, 600, 10, RED);
            DrawText((ZSTR() << "LEVEL: " << currentLevel << " / " << std::ssize(levelFiles)).str().c_str(), 10, 610, 10, RED);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
}

void Game::drawSprite(raylib::Vector2 worldPosition, const raylib::Texture2D& sprite, raylib::Vector2 spriteOrigin, bool horizontalMirror) const {
    raylib::Vector2 screenPosition;
    if (!horizontalMirror) {
        screenPosition = worldToScreen(worldPosition - spriteOrigin);
        sprite.Draw(screenPosition);
    }
    else {
        screenPosition = worldToScreen(worldPosition - raylib::Vector2(sprite.GetSize().x - spriteOrigin.x, spriteOrigin.y));
        sprite.Draw(raylib::Rectangle { raylib::Vector2::Zero(), raylib::Vector2{-1.0f, 1.0f} * sprite.GetSize() }, raylib::Rectangle { screenPosition, sprite.GetSize() });
    }
}

void gamepadTest()
{
    raylib::Texture2D texXboxPad("Menus/xbox.png");
    raylib::Gamepad gamepad(0);

    if (gamepad.IsAvailable())
    {
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
}
