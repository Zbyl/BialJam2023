#include "Menu.h"

#include "Game.h"

#include <vector>
#include <tuple>
#include <functional>

#include "raygui.h"


void Menu::update() {
    if (!inMenu && game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
        show(true);
        return;
    }

    if (inMenu && game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
        show(false);
        return;
    }
}

void Menu::draw() {
    if (!inMenu) {
        return;
    }

    if (useFuthark) {
        GuiSetFont(futharkFont);
    } else {
        GuiSetFont(GetFontDefault());
    }

    float buttonWidth = 140;
    float buttonX = menuRectangle.x + (menuRectangle.width - buttonWidth) / 2;
    auto yPosition = menuRectangle.y;

    std::vector< std::tuple< std::function<void()>, std::function<void()> > > items;
    auto addItem = [&items, &yPosition](std::tuple< std::function<void()>, std::function<void()> > item) {
        items.push_back(item);
        yPosition += 50.0f;
    };

    if (game.gameState == GameState::START_SCREEN)
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_PLAYER_PLAY, "Start game")); },
            [=, this]() { game.startLevel(0); },
        });
    if (game.gameState == GameState::LEVEL)
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_UNDO_FILL, "Back to game")); },
            [=, this]() { show(false); },
        });
    if ((game.gameState == GameState::LEVEL) || (game.gameState == GameState::LEVEL_DIED))
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_REDO_FILL, "Restart level")); },
            [=, this]() { game.restartLevel(); },
        });
    if (game.gameState != GameState::START_SCREEN)
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_REREDO_FILL, "Restart game")); },
            [=, this]() { game.restartGame(); },
        });
    if (false)
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_FILE_SAVE, "Save")); },
            [=, this]() {},
        });
    if (false)
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_TARGET_MOVE_FILL, "Show controls")); },
            [=, this]() {},
        });
    addItem({
        [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_CURSOR_SCALE, game.window.IsFullscreen() ? "Exit full screen" : "Full screen")); },
        [=, this]() { game.window.ToggleFullscreen(); },
    });
    addItem({
        [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_GEAR_BIG, useFuthark ? "To English" : "To Futhark")); },
        [=, this]() {
            useFuthark = !useFuthark;
            game.reloadScenes(useFuthark, true);
        },
        });
    addItem({
        [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_EXIT, "Quit")); },
        [=, this]() { game.shouldQuit = true; },
    });

    int numItems = std::ssize(items);

    auto currentYAxisValue = game.gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y);

    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_LEFT_FACE_UP) || (currentYAxisValue < -0.5f)) { // Left Up or D-Pad up
        if (!yaxisBlocked)
            focusedItem = (focusedItem + numItems - 1) % numItems;
        yaxisBlocked = true;
    }
    else
    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_LEFT_FACE_DOWN) || (currentYAxisValue > 0.5f)) { // Left Down or D-Pad down
        if (!yaxisBlocked)
            focusedItem = (focusedItem + 1) % numItems;
        yaxisBlocked = true;
    }
    else
        yaxisBlocked = false;

    for (int i = 0; i < numItems; ++i) {
        auto& [buttonFunc, actionFunc] = items[i];
        if (i == focusedItem) GuiSetState(STATE_FOCUSED);
        buttonFunc();
        if (i == focusedItem) GuiSetState(STATE_NORMAL);
    }

    auto& [buttonFunc, actionFunc] = items[focusedItem];
    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
        actionFunc();
    }
}

