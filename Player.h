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
    PlayerState state = PlayerState::GROUNDED;
    raylib::Vector2 position = { 0.0f, 0.0f };
    raylib::Vector2 velocity = { 0.0f, 0.0f };
    int facingDirection = 1;          ///< Player direction: 1 - right, -1 - left. Usually same as velocity.x, but sometimes not (when player is reversing, for example).

    float animTime = 0.0f; ///< Time for current animation.
    Animation* currentAnimation = &idleAnimation;
    Animation idleAnimation;
    Animation runAnimation;
    Animation jumpUpAnimation;
    Animation jumpDownAnimation;
    Animation hurtAnimation;
    Animation slideAnimation;
    Animation glideAnimation;
    Animation grabAnimation;

    float landMaxSpeed;                     ///< Max speed on land (pixels per second).
    float landAcceleration;                 ///< Land acceleration (pixels per second).
    float landDeceleration;                 ///< Land deceleration (pixels per second). Drag when player is not accelerating.
    float landHardDeceleration;             ///< Deceleration (pixels per second) when player is accelerating in opposite direction to it's velocity.
    float airCorrectionAcceleration;        ///< Acceleration when falling.

    float jumpVelocity;                     ///< Vertical velocity to set when player holds jump button.
    float jumpAccelerationTime;             ///< After this time stop applying jumpVelocity.
    raylib::Vector2 wallKickVelocity;       ///< Velocity to set when player holds jump button after wall kick.
    float wallKickAccelerationTime;         ///< After this time stop applying wallKickVelocity.
    float jumpBackPenalty;                  ///< Multiplayer for velocity.x applied before jumping with direction pressed in opposite direction to movement direction.

    float gravity;                          ///< Gravity in pixels per second squared.
    float glidingGravity;                   ///< Gravity for gliding in pixels per second squared.
    float jumpSustainGravity;               ///< Gravity applied during jumpAccelerationTime.
    raylib::Vector2 wallKickSustainGravity; ///< Gravity and deceleration applied during wallKickAccelerationTime.
    float jumpButtonActiveTime;             ///< Jump button is considered pressed for this amount of time after initial press (even if not held any more).
    raylib::Rectangle hitbox;               ///< Hitbox of the player.
    raylib::Rectangle cameraWindow;         ///< Fractions of the screen palyer must be in, unless level border doesn't allow it. @todo Should be in Game, but no time...

    float jumpButtonLastPressTime = -10.0f; ///< Time when user last pressed jump button.
    float jumpStartTime = -10.0f;           ///< Time when user started jumping/wall_kicking.

    int wallKickDirection = -1;             ///< Direction of the wall kick: 1 is right, -1 is left.
    int grabDirection = -1;                 ///< Where the wall player is grabbing is: 1 is right, -1 is left.

    bool jumpButtonBlocked = false;         ///< Used to disable reacting to a held jump button.
    bool jumpButtonOwned = false;           ///< Used to mark that this press of jump button was already "used". Different from jumpButtonBlocked because of jumpButtonActiveTime. @todo Maybe clean up.

    raylib::Sound jumpSfx;
    raylib::Sound groundSfx;

public:
    Player(Game& game)
        : game(game)
        , jumpSfx("Sounds/541210__eminyildirim__combat-whoosh.wav")
        , groundSfx("Sounds/stomp.wav")
    {
        idleAnimation.load("Graphics/Player/player-idle.json");
        runAnimation.load("Graphics/Player/player-run.json");
        jumpUpAnimation.load("Graphics/Player/player-jump-up.json");
        jumpDownAnimation.load("Graphics/Player/player-jump-down.json");
        hurtAnimation.load("Graphics/Player/player-hurt.json");
        slideAnimation.load("Graphics/Player/player-slide.json");
        glideAnimation.load("Graphics/Player/player-glide.json");
        grabAnimation.load("Graphics/Player/player-grab.json");
        load();
    }

    void setInitialState(raylib::Vector2 initialPosition) {
        state = PlayerState::GROUNDED;
        position = initialPosition;
        velocity = raylib::Vector2::Zero();
        facingDirection = 1;
        animTime = 0.0f;
        currentAnimation = &idleAnimation;

        jumpButtonLastPressTime = -10.0f;
        jumpStartTime = -10.0f;

        wallKickDirection = -1;
        grabDirection = -1;

        jumpButtonBlocked = false;
        jumpButtonOwned = false;
    }

    void update();
    void step(float timeDelta);
    void draw();
    void load();
};
