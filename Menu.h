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

public:
    Menu(Game& game) : game(game) {}

    bool isInMenu() const { return inMenu; }
    void setInMenu(bool inMenu) { if (this->inMenu == inMenu) return; this->inMenu = inMenu; if (inMenu) focusedItem = 0; }
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

