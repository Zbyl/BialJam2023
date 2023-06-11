#pragma once


#include "raylib-cpp.hpp"


class Game;


class Menu {
private:
    Game& game;
    bool inMenu = false;

    int focusedItem = 0;
    bool yaxisBlocked = false;  // We block Y axis after input to avoid registering continuous presses.

    raylib::Rectangle menuRectangle = { 0.0f, 0.0f, 1000.0f, 500.0f };
    raylib::Font futharkFont;

public:
    bool useFuthark = false;

public:
    Menu(Game& game) : game(game), futharkFont("Graphics/Fonts/pixelder_futhark.png") {}

    bool isInMenu() const { return inMenu; }
    void setInMenu(bool inMenu) { this->inMenu = inMenu; focusedItem = 0; }
    void setMenuRectangle(raylib::Rectangle menuRectangle) { this->menuRectangle = menuRectangle; }

    void show(bool doShow)
    {
        inMenu = doShow;
        focusedItem = 0;
        yaxisBlocked = false;
    }

    void update();
    void draw();
};

