#include "Game.h"

#include "Utilities.h"

#include "zstr.h"
#include "zerrors.h"


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

        auto& futFont = menu.useFuthark ? menu.futharkFont : hudFont;

        hudCollectible.position = raylib::Vector2{ startX, startY };
        hudCollectible.update();
        hudFont.DrawText((ZSTR() << collectedCount << " / " << totalCount).str(), raylib::Vector2{ startX + 30.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
        futFont.DrawText(toUpperEx(menu.useFuthark, "w poziomie"), raylib::Vector2{ startX + 140.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);

        startX = 930.0f;
        startY = 250.0f;
        hudCollectible.position = raylib::Vector2{ startX, startY };
        hudCollectible.update();
        hudFont.DrawText((ZSTR() << totalCollected << " / " << totalAvailable).str(), raylib::Vector2{ startX + 30.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
        futFont.DrawText(toUpperEx(menu.useFuthark, "w sumie"), raylib::Vector2{ startX + 140.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
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
    totalAvailable = 0;
    startScreen.startScene();
    currentLevel = 0;
    currentEpisode.clear();
}

void Game::restartLevel() {
    startLevel(currentLevel);
}

void Game::startLevel(int levelIndex) {
    menu.setInMenu(false);
    menu.setMenuRectangle({ 0.0f, 200.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight) });
    gameState = GameState::LEVEL;
    currentLevel = levelIndex;
    ZASSERT(episodes.contains(currentEpisode));
    ZASSERT(currentLevel < std::ssize(episodes.at(currentEpisode)));
    level.load(episodes.at(currentEpisode)[currentLevel]);
    level.startLevel();
    cameraPosition = player.position;
    cameraUpdate();
    waitUntilJumpNotPressed = true;
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
        if (currentLevel < std::ssize(episodes.at(currentEpisode)) - 1) {
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

void Game::drawFrame()
{
#if 0
    raylib::Vector2 hitboxVelocity = { 0, 0 };
    raylib::Rectangle hitbox = { 0, 0, 0, 0 };
    hitbox.SetPosition(player.position);
#endif

    if (!isInputDown(InputButton::JUMP)) { // A
        waitUntilJumpNotPressed = false;
    }

    if (debug) {
        if (gamepad.IsButtonPressed(GAMEPAD_BUTTON_MIDDLE_LEFT)) {
            player.state = PlayerState::GROUNDED;
            player.position = level.playerStartPosition;
            player.velocity = raylib::Vector2::Zero();
            player.load();
        }
    }

    if (IsKeyPressed(KEY_O))
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

            if (level.furharkTrigger.CheckCollision(player.position)) {
                level.setShowFuthark();
            }

            if (player.state == PlayerState::GROUNDED) {
                auto playerTile = level.getTileWorld(player.position + raylib::Vector2(0, level.tileSize / 2.0f)).value_or(TileType::EMPTY);
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
                if (isInputPressed(InputButton::MENU_ACTION)) { // A
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
                if (!menu.isInMenu() && isInputPressed(InputButton::MENU_ACTION)) { // A
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
                if (isInputPressed(InputButton::MENU_ACTION)) { // A
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
#if 0
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
#endif

        DrawText((ZSTR() << "GAME STATE: " << to_string(gameState)).str().c_str(), 10, 600, 10, RED);
        DrawText((ZSTR() << "LEVEL: " << currentEpisode << " " << currentLevel << " / " << (episodes.contains(currentEpisode) ? std::ssize(episodes.at(currentEpisode)) : -1)).str().c_str(), 10, 610, 10, RED);
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

void Game::mainLoop()
{
    // Main game loop, detect window close button, ESC key or programmatic quit.
    while (!window.ShouldClose() && !shouldQuit)
    {
        drawFrame();
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

void Game::reloadScenes(bool useFuthark, bool reloadHack) {
    startScreen.load("Scenes/StartScreen.json", useFuthark, reloadHack);
    deadScreen.load("Scenes/DeadScreen.json", useFuthark, reloadHack);
    levelEndScreen.load("Scenes/LevelEndScreen.json", useFuthark, reloadHack);
    gameEndScreen.load("Scenes/GameEndScreen.json", useFuthark, reloadHack);
    startScreen.startScene(reloadHack); // @hack
}

bool Game::isInputDown(InputButton button) const {
    switch (button) {
        case InputButton::MENU: return IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_GRAVE) || gamepad.IsButtonDown(GAMEPAD_BUTTON_MIDDLE_RIGHT);
        case InputButton::MENU_UP: return IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_UP) || (gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y) < -0.5f);
        case InputButton::MENU_DOWN: return IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S) || gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_DOWN) || (gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y) > 0.5f);
        case InputButton::MENU_ACTION: return IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER) || gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

        case InputButton::LEFT: return IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) || gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_LEFT) || (gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_X) < -0.5f);
        case InputButton::RIGHT: return IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || gamepad.IsButtonDown(GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || (gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_X) > 0.5f);
        case InputButton::JUMP: return IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE) || gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

        default:
            return false;
    }
}

bool Game::isInputPressed(InputButton button) const {
    switch (button) {
        case InputButton::MENU: return IsKeyPressed(KEY_GRAVE) || gamepad.IsButtonPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
        case InputButton::MENU_ACTION: return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

        //case InputButton::JUMP: return IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE) || gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

        default:
            ZASSERT(false) << "Button not supported for isInputPressed: " << static_cast<int>(button);
    }
}

void Game::load(const std::string& levelFile) {
    episodes.clear();

    // Custom data
    auto jsonText = loadTextFile(levelFile);
    auto json = nlohmann::json::parse(jsonText);
    auto basePath = std::filesystem::path(levelFile).parent_path();

    for (auto episode : json["episodes"]) {
        auto episodeName = episode["name"].get<std::string>();
        for (auto levelFile : episode["levels"]) {
            auto levelPath = levelFile.get<std::string>();
            episodes[episodeName].emplace_back((basePath / levelPath).string());
        }
    }
}
