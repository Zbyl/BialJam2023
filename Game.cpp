#include "Game.h"

#include "Utilities.h"

#include "zstr.h"
#include "zerrors.h"

#include <box2d/box2d.h>
#include <box2d/b2_time_of_impact.h>
#include <PlayRho/PlayRho.hpp>
#include <PlayRho/Collision/Distance.hpp>
#include <PlayRho/Collision/Manifold.hpp>
#include <PlayRho/Collision/WorldManifold.hpp>


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
        futFont.DrawText(textForFont(!menu.useFuthark, false, U"w poziomie"), raylib::Vector2{ startX + 140.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);

        startX = 930.0f;
        startY = 250.0f;
        hudCollectible.position = raylib::Vector2{ startX, startY };
        hudCollectible.update();
        hudFont.DrawText((ZSTR() << totalCollected << " / " << totalAvailable).str(), raylib::Vector2{ startX + 30.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
        futFont.DrawText(textForFont(!menu.useFuthark, false, U"w sumie"), raylib::Vector2{ startX + 140.0f, startY - 20.0f }, 40.0f, 1.0f, GOLD);
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

class BoxWorld {
public:
    static const float worldToPhysicsRatio;    // 32 world pixels are 1 physics unit. Improves physics stability.

public:
    // Testing and debug stuff.
    b2Transform playerTransform;
    b2Transform groundTransform;
    b2PolygonShape playerBox;
    b2PolygonShape groundBox;

    b2Vec2 chainPoints[4];
    b2ChainShape chain;
    b2EdgeShape edge;
    b2Transform chainTransform;

    playrho::d2::Transformation prPlayerTransform { { 0.0f, 0.9f } };
    playrho::d2::Transformation prGroundTransform{ { 0.0f, -10.f } };
    playrho::d2::Transformation prChainTransform{ { 0.0f, 0.f } };
    playrho::d2::Shape prPlayerBox { playrho::d2::PolygonShapeConf{}.SetAsBox(1.0f, 1.0f) };
    playrho::d2::Shape prGroundBox { playrho::d2::PolygonShapeConf{}.SetAsBox(40.0f, 10.0f) };
    //playrho::d2::Shape prChain{ playrho::d2::ChainShapeConf{}.Add({-5.0f, 0.0f}).Add({-1.0f, 1.0f}).Add({1.0f, 1.0f}).Add({5.0f, 0.0f}).Add({-5.0f, 0.0f}) };
    playrho::d2::Shape prChain{ playrho::d2::ChainShapeConf{}.Add({5.0f, 0.0f}).Add({1.0f, 1.0f}).Add({-1.0f, 1.0f}).Add({-5.0f, 0.0f}).Add({5.0f, 0.0f}) };
};

const float BoxWorld::worldToPhysicsRatio = 32.0f;    // 32 world pixels are 1 physics unit. Improves physics stability.

void Game::initBoxWorld() {
    boxWorld = std::make_shared<BoxWorld>();

    boxWorld->playerTransform.Set({ 0.0f, 0.9f }, 0.0f);
    boxWorld->groundTransform.Set({ 0.0f, -10.0f }, 0.0f);
    boxWorld->playerBox.SetAsBox(1.0f, 1.0f);
    boxWorld->groundBox.SetAsBox(40.0f, 10.0f);

    if (false) {
        boxWorld->chainPoints[0].Set(-5.0f, 0.0f);
        boxWorld->chainPoints[1].Set(-1.0f, 1.0f);
        boxWorld->chainPoints[2].Set(1.0f, 1.0f);
        boxWorld->chainPoints[3].Set(5.0f, 0.0f);
    } else {
        boxWorld->chainPoints[0].Set(5.0f, 0.0f);
        boxWorld->chainPoints[1].Set(1.0f, 1.0f);
        boxWorld->chainPoints[2].Set(-1.0f, 1.0f);
        boxWorld->chainPoints[3].Set(-5.0f, 0.0f);
    }

    boxWorld->chain.CreateLoop(boxWorld->chainPoints, 4);
    boxWorld->chainTransform.Set({ 0.0f, 0.0f }, 0.0f);
}

auto Game::collideBoxes(raylib::Rectangle object, raylib::Vector2 velocity, raylib::Rectangle blocker) -> std::tuple<bool, float> {
    auto rectToBox = [](raylib::Rectangle rect) -> std::tuple<Vector2, b2PolygonShape> {
        b2PolygonShape box;
        box.SetAsBox(rect.width / (2.0f * BoxWorld::worldToPhysicsRatio), rect.height / (2.0f * BoxWorld::worldToPhysicsRatio));
        auto objectCenter = rect.GetPosition() + rect.GetSize() * raylib::Vector2(0.5f, 0.5f);
        auto objectPhysCenter = objectCenter / raylib::Vector2(BoxWorld::worldToPhysicsRatio, BoxWorld::worldToPhysicsRatio);
        return { objectPhysCenter, box };
    };

    auto [objectCenter, objectBox] = rectToBox(object);
    auto [blockerCenter, blockerBox] = rectToBox(blocker);

    b2Sweep sweepA;
    sweepA.c0.Set(objectCenter.x, objectCenter.y);
    sweepA.a0 = 0.0f;
    sweepA.c.Set(objectCenter.x + velocity.x / BoxWorld::worldToPhysicsRatio, objectCenter.y + velocity.y / BoxWorld::worldToPhysicsRatio);
    sweepA.a = sweepA.a0;
    sweepA.localCenter.SetZero();

    b2Sweep sweepB;
    sweepB.c0.Set(blockerCenter.x, blockerCenter.y);
    sweepB.a0 = 0.0f;
    sweepB.c = sweepB.c0;
    sweepB.a = 0.0f;
    sweepB.localCenter.SetZero();

    b2TOIInput input;
    input.proxyA.Set(&objectBox, 0);
    input.proxyB.Set(&blockerBox, 0);
    input.sweepA = sweepA;
    input.sweepB = sweepB;
    input.tMax = 1.0f;

    b2TOIOutput output;
    b2TimeOfImpact(&output, &input);
    if (output.state == b2TOIOutput::e_overlapped)
        return { true, 0.0f };
    if (output.state == b2TOIOutput::e_touching)
        return { true, output.t };

    return { false, 1.0f };
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
                if (!menu.isInMenu() && isInputPressed(InputButton::MENU_ACTION)) { // A
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
        if (false) {
            auto cameraScreenPosition = worldToScreen(cameraPosition);
            raylib::Rectangle wndRect = { player.cameraWindow.x * screenWidth, player.cameraWindow.y * screenHeight, player.cameraWindow.width * screenWidth, player.cameraWindow.height * screenHeight };
            DrawRectangleLines(wndRect.x, wndRect.y, wndRect.width, wndRect.height, BLUE);
        }

        // Debug Box collisions.
        if (true)
        {
            raylib::Rectangle blocker;
            blocker.SetPosition(cameraPosition);
            blocker.SetSize(32.0f, 32.0f);

            raylib::Rectangle player;
            player.SetPosition(cameraPosition + raylib::Vector2(40.0f, 0.0f));
            player.SetSize(32.0f, 32.0f);

            raylib::Vector2 velocity;
            velocity.x = gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_X) * 20.0f;
            velocity.y = gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_Y) * 20.0f;

            auto [collision, timeOfImpact] = collideBoxes(player, velocity, blocker);

            raylib::Rectangle destPlayer = player;
            destPlayer.SetPosition(player.GetPosition() + velocity);

            raylib::Rectangle newPlayer = player;
            newPlayer.SetPosition(player.GetPosition() + velocity * timeOfImpact);

            drawScreenRect(blocker, BLUE);
            drawScreenRect(player, GREEN);
            drawScreenRect(destPlayer, YELLOW);
            drawScreenRect(newPlayer, RED);

            DrawText((ZSTR() << "Collision: " << collision << " TOI: " << timeOfImpact).str().c_str(), 10, 300, 10, BLACK);
        }

        // Debug BoxWorld
        if (false)
        {

            boxWorld->playerTransform.p.x += gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_X) / 25.0f;
            boxWorld->playerTransform.p.y -= gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_Y) / 25.0f;

            auto physToScreen = [this](b2Vec2 phys) -> raylib::Vector2 {
                return { phys.x * BoxWorld::worldToPhysicsRatio + screenWidth / 2.0f, -phys.y * BoxWorld::worldToPhysicsRatio + screenHeight / 2.0f };
            };

            auto physSizeToScreen = [this](b2Vec2 physSize) -> raylib::Vector2 {
                return { physSize.x * BoxWorld::worldToPhysicsRatio, physSize.y * BoxWorld::worldToPhysicsRatio };
            };

            raylib::Rectangle{ physToScreen(boxWorld->playerTransform.p) - physSizeToScreen({ 1.0f, 1.0f}), physSizeToScreen({ 2.0f, 2.0f }) }.DrawLines(RED);
            raylib::Rectangle{ physToScreen(boxWorld->groundTransform.p) - physSizeToScreen({ 40.0f, 10.0f}), physSizeToScreen({ 80.0f, 20.0f }) }.DrawLines(BLUE);

            for (int i = 0; i < 4; ++i) {
                const auto& point0 = boxWorld->chainPoints[i];
                const auto& point1 = boxWorld->chainPoints[(i + 1) % 4];
                physToScreen(point0).DrawLine(physToScreen(point1), ORANGE);
            }

            bool overlap = b2TestOverlap(&boxWorld->groundBox, 0, &boxWorld->playerBox, 0, boxWorld->groundTransform, boxWorld->playerTransform);
            DrawText(overlap ? "OVERLAP" : "NO OVERLAP", 10, 300, 10, BLACK);

            {
                b2Manifold manifold;
                b2CollidePolygons(&manifold, &boxWorld->groundBox, boxWorld->groundTransform, &boxWorld->playerBox, boxWorld->playerTransform);
                b2WorldManifold worldManifold;
                worldManifold.Initialize(&manifold, boxWorld->groundTransform, boxWorld->groundBox.m_radius, boxWorld->playerTransform, boxWorld->playerBox.m_radius);

                for (int32 i = 0; i < manifold.pointCount; ++i)
                {
                    b2Vec2 point = worldManifold.points[i];
                    auto target = point + worldManifold.normal;
                    physToScreen(point).DrawLine(physToScreen(target), YELLOW);
                    physToScreen(target).DrawCircle(2.0f, BLUE);
                }

                b2Sweep sweepA;
                sweepA.c0.Set(boxWorld->groundTransform.p.x, boxWorld->groundTransform.p.y);
                sweepA.a0 = 0.0f;
                sweepA.c = sweepA.c0;
                sweepA.a = sweepA.a0;
                sweepA.localCenter.SetZero();

                b2Sweep sweepB;
                sweepB.c0.Set(boxWorld->playerTransform.p.x, boxWorld->playerTransform.p.y);
                sweepB.a0 = 0.0f;
                sweepB.c.Set(0.0f, 0.0f);
                sweepB.a = 0.0f;
                sweepB.localCenter.SetZero();

                b2TOIInput input;
                input.proxyA.Set(&boxWorld->groundBox, 0);
                input.proxyB.Set(&boxWorld->playerBox, 0);
                input.sweepA = sweepA;
                input.sweepB = sweepB;
                input.tMax = 1.0f;

                b2TOIOutput output;
                b2TimeOfImpact(&output, &input);
                DrawText((ZSTR() << "TOI: " << output.t << " state: " << output.state).str().c_str(), 10, 310, 10, BLACK);
            }

            for (int childIndex = 0; childIndex < boxWorld->chain.GetChildCount(); ++childIndex)
            {
                b2Manifold manifold;
                boxWorld->chain.GetChildEdge(&boxWorld->edge, childIndex);
                b2CollideEdgeAndPolygon(&manifold, &boxWorld->edge, boxWorld->chainTransform, &boxWorld->playerBox, boxWorld->playerTransform);
                b2WorldManifold worldManifold;
                worldManifold.Initialize(&manifold, boxWorld->chainTransform, boxWorld->edge.m_radius, boxWorld->playerTransform, boxWorld->playerBox.m_radius);

                for (int32 i = 0; i < manifold.pointCount; ++i)
                {
                    b2Vec2 point = worldManifold.points[i];
                    auto target = point + worldManifold.normal;
                    physToScreen(point).DrawLine(physToScreen(target), YELLOW);
                    physToScreen(target).DrawCircle(2.0f, BLUE);
                }
            }
        }
        if (false)
        {
            auto physToScreen = [this](b2Vec2 phys) -> raylib::Vector2 {
                return { phys.x * BoxWorld::worldToPhysicsRatio + screenWidth / 2.0f, -phys.y * BoxWorld::worldToPhysicsRatio + screenHeight / 2.0f };
            };

            for (int i = 0; i < 4; ++i) {
                const auto& point0 = boxWorld->chainPoints[i];
                const auto& point1 = boxWorld->chainPoints[(i + 1) % 4];
                physToScreen(point0).DrawLine(physToScreen(point1), ORANGE);
            }
        }

        // Debug PlayRho
        if (false)
        {

            boxWorld->prPlayerTransform.p[0] += gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_X) / 25.0f;
            boxWorld->prPlayerTransform.p[1] -= gamepad.GetAxisMovement(GAMEPAD_AXIS_RIGHT_Y) / 25.0f;

            auto physToScreen = [this](playrho::Vec2 phys) -> raylib::Vector2 {
                return { phys[0] * BoxWorld::worldToPhysicsRatio + screenWidth / 2.0f, -phys[1] * BoxWorld::worldToPhysicsRatio + screenHeight / 2.0f };
            };

            auto physSizeToScreen = [this](playrho::Vec2 physSize) -> raylib::Vector2 {
                return { physSize[0] * BoxWorld::worldToPhysicsRatio, physSize[1] * BoxWorld::worldToPhysicsRatio };
            };

            raylib::Rectangle{ physToScreen(boxWorld->prPlayerTransform.p) - physSizeToScreen({ 1.0f, 1.0f}), physSizeToScreen({ 2.0f, 2.0f }) }.DrawLines(RED);
            raylib::Rectangle{ physToScreen(boxWorld->prGroundTransform.p) - physSizeToScreen({ 40.0f, 10.0f}), physSizeToScreen({ 80.0f, 20.0f }) }.DrawLines(BLUE);

            {
                auto overlap = TestOverlap(GetChild(boxWorld->prGroundBox, 0), boxWorld->prGroundTransform, GetChild(boxWorld->prPlayerBox, 0), boxWorld->prPlayerTransform);
                DrawText((ZSTR() << "OVERLAP AREA " << overlap).str().c_str(), 10, 300, 10, BLACK);
            }

            {
                auto overlap = TestOverlap(GetChild(boxWorld->prChain, 0), boxWorld->prChainTransform, GetChild(boxWorld->prPlayerBox, 0), boxWorld->prPlayerTransform);
                DrawText((ZSTR() << "CHAIN OVERLAP AREA " << overlap).str().c_str(), 10, 310, 10, BLACK);
            }

            {
                const auto manifold = CollideShapes(GetChild(boxWorld->prGroundBox, 0), boxWorld->prGroundTransform, GetChild(boxWorld->prPlayerBox, 0), boxWorld->prPlayerTransform);

                auto worldManifold = playrho::d2::GetWorldManifold(manifold, boxWorld->prGroundTransform, GetVertexRadius(boxWorld->prGroundBox, 0), boxWorld->prPlayerTransform, GetVertexRadius(boxWorld->prPlayerBox, 0));
                for (int32 i = 0; i < worldManifold.GetPointCount(); ++i)
                {
                    auto point = worldManifold.GetPoint(i);
                    physToScreen(point).DrawLine(physToScreen(point + playrho::Length2 { worldManifold.GetNormal()[0], worldManifold.GetNormal()[1] }), YELLOW);
                }
            }
            for (int childIndex = 0; childIndex < GetChildCount(boxWorld->prChain); ++childIndex)
            {
                const auto manifold = CollideShapes(GetChild(boxWorld->prChain, childIndex), boxWorld->prChainTransform, GetChild(boxWorld->prPlayerBox, 0), boxWorld->prPlayerTransform);

                auto worldManifold = playrho::d2::GetWorldManifold(manifold, boxWorld->prChainTransform, GetVertexRadius(boxWorld->prChain, childIndex), boxWorld->prPlayerTransform, GetVertexRadius(boxWorld->prPlayerBox, 0));
                for (int32 i = 0; i < worldManifold.GetPointCount(); ++i)
                {
                    auto point = worldManifold.GetPoint(i);
                    auto target = point + playrho::Length2{ worldManifold.GetNormal()[0], worldManifold.GetNormal()[1] };
                    physToScreen(point).DrawLine(physToScreen(target), GREEN);
                    physToScreen(target).DrawCircle(2.0f, BLUE);
                }
            }
        }

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

        flushScreenRects();
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
    shouldQuit = shouldQuit;    // Point for a berakpoint.
}

void Game::mainLoop()
{
    initBoxWorld();

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
        auto episodeName = loadUnicodeStringFromJson(episode, "name");
        for (auto levelFile : episode["levels"]) {
            auto levelPath = levelFile.get<std::string>();
            episodes[episodeName].emplace_back((basePath / levelPath).string());
        }
    }
}

Game::~Game() {
}

void Game::drawScreenRect(raylib::Rectangle rect, Color color, int layer) {
    screenRects.emplace_back(rect, color, layer);
}

void Game::flushScreenRects() {
    std::sort(screenRects.begin(), screenRects.end(), [](const auto& tup0, const auto& tup1) { return std::get<2>(tup0) < std::get<2>(tup1); });

    for (auto& [screenRect, color, layer] : screenRects) {
        auto position = worldToScreen(screenRect.GetPosition());
        DrawRectangleLines(position.x, position.y, screenRect.width, screenRect.height, color);
    }
    screenRects.clear();
}
