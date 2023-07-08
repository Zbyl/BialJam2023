#include "Menu.h"

#include "Game.h"
#include "Utilities.h"

#include <vector>
#include <tuple>
#include <functional>

#include "raygui.h"

#include "zstr.h"


Menu::Menu(Game& game)
    : game(game)
    , charset(loadCharset("Graphics/Fonts/charset.txt"))
    , menuFont("Graphics/Fonts/zekton-free.rg-regular.otf", 16, charset.data(), std::ssize(charset))
    , futharkFont("Graphics/Fonts/futhark.png")
{}

void Menu::update() {
    if (!inMenu && game.isInputPressed(InputButton::MENU)) {
        show(true);
        return;
    }

    if (inMenu && game.isInputPressed(InputButton::MENU)) {
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
        GuiSetFont(menuFont);
    }

    if (game.gameState != GameState::START_SCREEN) {
        auto numEpisodes = game.episodes.contains(game.currentEpisode) ? std::ssize(game.episodes.at(game.currentEpisode)) : -1;
        auto& curFont = (useFuthark ? futharkFont : menuFont);
        curFont.DrawText((ZSTR() << "LEVEL: " << textForFont(!useFuthark, !useFuthark, game.currentEpisode) << ": " << textForFont(!useFuthark, !useFuthark, game.level.levelDescription) << " " << game.currentLevel << " / " << numEpisodes).str().c_str(), {10, 10}, curFont.baseSize, 1.0f, BLUE);
    }

#if defined(PLATFORM_WEB)
    const bool webBuild = true;
#else
    const bool webBuild = false;
#endif

    float buttonWidth = 140;
    float buttonX = menuRectangle.x + (menuRectangle.width - buttonWidth) / 2;
    auto yPosition = menuRectangle.y;

    std::vector< std::tuple< std::function<void()>, std::function<void()> > > items;
    auto addItem = [&items, &yPosition](std::tuple< std::function<void()>, std::function<void()> > item) {
        items.push_back(item);
        yPosition += 50.0f;
    };

    if (episodeSelect) {
        for (const auto& [episodeName, levelFiles] : game.episodes) {
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, textForFont(!useFuthark, !useFuthark, episodeName).c_str()); },
                [=, this]() { game.currentEpisode = episodeName; game.startLevel(0); },
            });
        }
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_UNDO_FILL, textForFont(!useFuthark, !useFuthark, U"Wstecz").c_str())); },
            [=, this]() { episodeSelect = false; focusedItem = 0; },
        });
    }
    else {
        if (game.gameState == GameState::START_SCREEN)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_PLAYER_PLAY, textForFont(!useFuthark, !useFuthark, U"Nowa gra").c_str())); },
                [=, this]() { game.load("Levels/Levels.json"); episodeSelect = true; focusedItem = 0; },
            });
        if (game.gameState == GameState::LEVEL)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_UNDO_FILL, textForFont(!useFuthark, !useFuthark, U"Wróć do gry").c_str())); },
                [=, this]() { show(false); },
            });
        if ((game.gameState == GameState::LEVEL) || (game.gameState == GameState::LEVEL_DIED))
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_REDO_FILL, textForFont(!useFuthark, !useFuthark, U"Restart poziomu").c_str())); },
                [=, this]() { game.restartLevel(); },
            });
        if (game.gameState == GameState::LEVEL)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_PLAYER_NEXT, textForFont(!useFuthark, !useFuthark, U"Zakończ poziom").c_str())); },
                [=, this]() { game.endLevel(false); },
            });
        if (game.gameState != GameState::START_SCREEN)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_REREDO_FILL, textForFont(!useFuthark, !useFuthark, U"Restart gry").c_str())); },
                [=, this]() { game.restartGame(); },
            });
        if (false)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_FILE_SAVE, textForFont(!useFuthark, !useFuthark, U"Zapisz").c_str())); },
                [=, this]() {},
            });
        if (false)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_TARGET_MOVE_FILL, textForFont(!useFuthark, !useFuthark, U"Sterowanie").c_str())); },
                [=, this]() {},
            });
        if (!webBuild)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_CURSOR_SCALE, textForFont(!useFuthark, !useFuthark, game.window.IsFullscreen() ? U"W oknie" : U"Pełny ekran").c_str())); },
                [=, this]() { game.window.ToggleFullscreen(); },
            });
        addItem({
            [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_GEAR_BIG, textForFont(!useFuthark, !useFuthark, useFuthark ? U"Polish" : U"Futhark").c_str())); },
            [=, this]() {
                useFuthark = !useFuthark;
                game.reloadScenes(useFuthark, true);
            },
            });
        if (!webBuild)
            addItem({
                [=, this]() { GuiButton(raylib::Rectangle { buttonX, yPosition, buttonWidth, 30 }, GuiIconText(ICON_EXIT, textForFont(!useFuthark, !useFuthark, U"Wyjdź z gry").c_str())); },
                [=, this]() { game.shouldQuit = true; },
            });
    }

    auto numItems = static_cast<int>(std::ssize(items));

    if (game.isInputDown(InputButton::MENU_UP)) {
        if (!yaxisBlocked)
            focusedItem = (focusedItem + numItems - 1) % numItems;
        yaxisBlocked = true;
    }
    else
    if (game.isInputDown(InputButton::MENU_DOWN)) {
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
    if (game.isInputPressed(InputButton::MENU_ACTION)) { // A
        actionFunc();
    }
}

