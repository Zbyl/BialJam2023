#pragma once

#include "Animation.h"


#include "raylib-cpp.hpp"
#include "zerrors.h"


class Game;


enum class PlayerState {
    GROUNDED,
    JUMPING,
    WALL_KICK,
    FALLING,
    GRABBING,
    GLIDING,
};

inline std::string to_string(PlayerState state) {
    switch (state) {
        case PlayerState::GROUNDED: return "GROUNDED";
        case PlayerState::JUMPING: return "JUMPING";
        case PlayerState::WALL_KICK: return "WALL_KICK";
        case PlayerState::FALLING: return "FALLING";
        case PlayerState::GRABBING: return "GRABBING";
        case PlayerState::GLIDING: return "GLIDING";
    }
    ZASSERT(false);
}

class Player {
private:
    Game& game;

public:
    PlayerState state;
    raylib::Vector2 position;
    raylib::Vector2 velocity;
    int facingDirection = 1;          ///< Player direction: 1 - right, -1 - left. Usually same as velocity.x, but sometimes not (when player is reversing, for example).

    float animTime = 0.0f; ///< Time for current animation.
    Animation runAnimation;

    float landMaxSpeed;                     ///< Max speed on land (pixels per second).
    float landAcceleration;                 ///< Land acceleration (pixels per second).
    float landDeceleration;                 ///< Land deceleration (pixels per second). Drag when player is not accelerating.
    float airCorrectionAcceleration;        ///< Acceleration when falling.

    float jumpVelocity;                     ///< Vertical velocity to set when player holds jump button.
    float jumpAccelerationTime;             ///< After this time stop applying jumpVelocity.
    raylib::Vector2 wallKickVelocity;       ///< Velocity to set when player holds jump button after wall kick.
    float wallKickAccelerationTime;         ///< After this time stop applying wallKickVelocity.

    float gravity;                          ///< Gravity in pixels per second squared.
    float glidingGravity;                   ///< Gravity for gliding in pixels per second squared.
    float jumpSustainGravity;               ///< Gravity applied during jumpAccelerationTime.
    float jumpButtonActiveTime;             ///< Jump button is considered pressed for this amount of time after initial press (even if not held any more).
    raylib::Rectangle hitbox;               ///< Hitbox of the player.

    float jumpButtonLastPressTime = -10.0f; ///< Time when user last pressed jump button.
    float jumpStartTime = -10.0f;           ///< Time when user started jumping/wall_kicking.

    int wallKickDirection;                  ///< Direction of the wall kick: 1 is right, -1 is left.
    int grabDirection;                      ///< Where the wall player is grabbing is: 1 is right, -1 is left.

    raylib::Sound jumpSfx;

public:
    Player(Game& game)
        : game(game)
        , jumpSfx("Sounds/663831__efindlay__springy-jump.wav")
    {
        runAnimation.load("Graphics/Player/player-run.json");
        load();
    }

    void update();
    void load();
};
