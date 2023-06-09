
#include "Player.h"

#include "Game.h"


void Player::update() {
    animTime += game.levelTimeDelta;

    auto currentXAxisValue = game.gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_X);
    auto currentYAxisValue = game.gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y);

    int axisY = 0; // 1 is up, -1 is down.
    int axisX = 0; // 1 is right, -1 is left.
    auto buttonJump = false;
    auto buttonGrab = false;
    auto buttonGlide = false;

    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || (currentXAxisValue > 0.5f)) { // Left Right or D-Pad right
        axisX += 1;
    }
    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT) || (currentXAxisValue < -0.5f)) { // Left Left or D-Pad left
        axisX -= 1;
    }

    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_LEFT_FACE_UP) || (currentYAxisValue < -0.5f)) { // Left Up or D-Pad up
        axisY += 1;
    }
    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_LEFT_FACE_DOWN) || (currentYAxisValue > 0.5f)) { // Left Down or D-Pad down
        axisY -= 1;
    }


    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
        buttonJump = true;
    }

    if (game.gamepad.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
        buttonJump = true;
    }

    if (buttonJump) {
        game.sfx.Play();
    }

    position += raylib::Vector2(axisX, axisY) * 1.0f;

    auto [origin, image] = runAnimation.spriteForTime(animTime);
    auto screenPosition = game.worldToScreen(position + origin);
    image.Draw(screenPosition);
}
