#pragma once


class Game;


class Menu {
private:
    Game& game;
    bool inMenu = false;

    int focusedItem = 0;
    bool yaxisBlocked = false;  // We block Y axis after input to avoid registering continuous presses.

public:
    Menu(Game& game) : game(game) {}

    bool isInMenu() const { return inMenu; }

    void show(bool doShow)
    {
        inMenu = doShow;
        focusedItem = 0;
        yaxisBlocked = false;
    }

    void update();
};

