
#include "Player.h"

#include "Game.h"
#include "Utilities.h"

#include "zstr.h"

#include "nlohmann/json.hpp"

#include <cmath>


Player::Player(Game& game)
    : game(game)
    , jumpSfx("Sounds/541210__eminyildirim__combat-whoosh.wav")
    , groundSfx("Sounds/step.wav")
{
    idleAnimation.load(game.resourceCache, "Graphics/Player/player-idle.json");
    runAnimation.load(game.resourceCache, "Graphics/Player/player-run.json");
    jumpUpAnimation.load(game.resourceCache, "Graphics/Player/player-jump-up.json");
    jumpDownAnimation.load(game.resourceCache, "Graphics/Player/player-jump-down.json");
    hurtAnimation.load(game.resourceCache, "Graphics/Player/player-hurt.json");
    slideAnimation.load(game.resourceCache, "Graphics/Player/player-slide.json");
    glideAnimation.load(game.resourceCache, "Graphics/Player/player-glide.json");
    grabAnimation.load(game.resourceCache, "Graphics/Player/player-grab.json");
    load();
}

void Player::update() {
    animTime += game.levelTimeDelta;

    if (playerDead)
        return;

    auto steps = 20.0f;
    auto timeDelta = game.levelTimeDelta / steps;
    for (int i = 0; i < steps; ++i) {
        step(timeDelta);
    }

    DrawText((ZSTR() << "PLAYER STATE: " << to_string(state)).str().c_str(), 10, 10, 10, BLACK);
    DrawText((ZSTR() << "POS X: " << position.x << " Y: " << position.y).str().c_str(), 10, 20, 10, BLACK);
    DrawText((ZSTR() << "VEL X: " << velocity.x << " Y: " << velocity.y).str().c_str(), 10, 30, 10, BLACK);
    DrawText((ZSTR() << "FACING: " << facingDirection << " GRAB: " << grabDirection << " KICK: " << wallKickDirection).str().c_str(), 10, 40, 10, BLACK);
    if (jumpButtonOwned)
        DrawText("JUMP OWNED", 10, 70, 10, BLACK);
}

void Player::step(float timeDelta) {
    auto currentXAxisValue = game.gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_X);
    auto currentYAxisValue = game.gamepad.GetAxisMovement(GAMEPAD_AXIS_LEFT_Y);

    int axisY = 0; // 1 is up, -1 is up.
    int axisX = 0; // 1 is right, -1 is left.
    auto buttonJump = false;
    auto buttonGrab = false;
    auto buttonGlide = false;

    if (currentXAxisValue > 0.5f) {
        axisX += 1;
    }
    if (currentXAxisValue < -0.5f) {
        axisX -= 1;
    }

    if (currentYAxisValue < -0.5f) {
        axisY -= 1;
    }
    if (currentYAxisValue > 0.5f) {
        axisY += 1;
    }

    if (game.gamepad.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) { // A
        if (jumpButtonBlocked) {
            DrawText("JUMP BLOCKED", 10, 60, 10, BLACK);
        } else {
            DrawText("JUMP HELD", 10, 60, 10, BLACK);
            buttonJump = true;
            jumpButtonLastPressTime = game.levelTime;
        }
    } else {
        jumpButtonBlocked = false;
    }

    if (game.levelTime <= jumpButtonLastPressTime + jumpButtonActiveTime) {
        DrawText("JUMP HELD ARTIFICIALLY", 10, 50, 10, BLACK);
        buttonJump = true;
    }
    else {
        if (!jumpButtonBlocked) jumpButtonOwned = false;
    }

    // Check collisions and push back.
    auto [origin, image, sound] = runAnimation.spriteForTime(animTime); // @todo Using anim for hitbox is broken here.
    raylib::Rectangle currentHitbox = { position - origin + hitbox.GetPosition(), hitbox.GetSize() };
    auto [grounded, touchingCeiling, touchingWall, touchingWallDirection, moveDelta] = game.level.collisionDetection(currentHitbox, velocity);
    if (grounded || touchingCeiling) {
        velocity.y = 0.0f;
    }
    if (touchingWall) {
        if ((velocity.x > 0) && (touchingWallDirection == 1))
            velocity.x = 0.0f;
        if ((velocity.x < 0) && (touchingWallDirection == -1))
            velocity.x = 0.0f;
    }

    auto oldState = state;
    if (grounded) {
        state = PlayerState::GROUNDED;
        if (buttonJump) {
            state = PlayerState::JUMPING;
        }
    }
    else
    {
        if (state == PlayerState::GROUNDED) {
            state = PlayerState::FALLING;
        }
        if (buttonGlide) {
            state = PlayerState::GLIDING;
        }
        if (touchingWall && buttonGrab) {
            state = PlayerState::GRABBING;
            grabDirection = touchingWallDirection;
            facingDirection = -touchingWallDirection;
        }
        if (touchingWall && buttonJump && !jumpButtonOwned) {
            state = PlayerState::WALL_KICK;
            wallKickDirection = -touchingWallDirection;
            facingDirection = -touchingWallDirection;
        }
    }

    if (((state == PlayerState::JUMPING) || (state == PlayerState::WALL_KICK) || (state == PlayerState::GLIDING)) && touchingCeiling) {
        state = PlayerState::FALLING;
    }

    if (((state == PlayerState::FALLING) || (state == PlayerState::GLIDING)) && grounded) {
        state = PlayerState::GROUNDED;
    }

    if (state == PlayerState::JUMPING) {
        if (oldState != state) {
            jumpSfx.Play();
            jumpStartTime = game.levelTime;
            jumpButtonOwned = true;
            if (std::signbit(static_cast<float>(axisX)) != std::signbit(velocity.x)) {
                velocity.x *= jumpBackPenalty;
            }
        }
        if (buttonJump && (game.levelTime <= jumpStartTime + jumpAccelerationTime)) {
            auto jumpTime = game.levelTime - jumpStartTime;
            velocity.y = -jumpVelocity;
            velocity.y += jumpSustainGravity * jumpTime * jumpTime;
        } else {
            jumpButtonBlocked = true;
            state = PlayerState::FALLING;
            if (buttonGlide) {
                state = PlayerState::GLIDING;
            }
        }
    }

    bool startedWallKick = false;

    if (state == PlayerState::WALL_KICK) {
        if (oldState != state) {
            jumpSfx.Play();
            jumpStartTime = game.levelTime;
            jumpButtonOwned = true;
        }
        if (buttonJump && (game.levelTime <= jumpStartTime + wallKickAccelerationTime)) {
            auto jumpTime = game.levelTime - jumpStartTime;
            velocity.y = -wallKickVelocity.y;
            velocity.y += wallKickSustainGravity.y * jumpTime * jumpTime;
            velocity.x = wallKickVelocity.x * wallKickDirection;
            velocity.x += wallKickSustainGravity.x * wallKickDirection * jumpTime * jumpTime;
        }
        else {
            jumpButtonBlocked = true;
            state = PlayerState::FALLING;
            if (buttonGlide) {
                state = PlayerState::GLIDING;
            }
        }
    }

    if ((state == PlayerState::JUMPING) || (state == PlayerState::FALLING) || (state == PlayerState::GLIDING) || ((state == PlayerState::WALL_KICK) && !startedWallKick)) {
        if (touchingWall && buttonGrab) {
            state = PlayerState::GRABBING;
            facingDirection = -touchingWallDirection;
        }
    }

    if ((state == PlayerState::JUMPING) || (state == PlayerState::FALLING)) {
        velocity.x += axisX * airCorrectionAcceleration * timeDelta;
        if (axisX != 0) {
            facingDirection = axisX;
        }
    }
    else
    if (state == PlayerState::GROUNDED) {
        if (axisX != 0) {
            if (std::signbit(static_cast<float>(axisX)) == std::signbit(velocity.x))
                velocity.x += axisX * landAcceleration * timeDelta;
            else
                velocity.x += axisX * landHardDeceleration * timeDelta;
            facingDirection = axisX;
        }
        else
            if (velocity.x > 0)
                velocity.x = std::max(0.0f, velocity.x - landDeceleration * timeDelta);
            else
                velocity.x = std::min(0.0f, velocity.x + landDeceleration * timeDelta);
    }

    if (state == PlayerState::FALLING) {
        velocity.y += gravity * timeDelta;
    }
    else
    if (state == PlayerState::GLIDING) {
        velocity.y += glidingGravity * timeDelta;
    }

    if ((oldState != state) && (state == PlayerState::GROUNDED)) {
        groundSfx.Play();
    }

    if (velocity.x > 0) velocity.x = std::min(velocity.x, landMaxSpeed);
    if (velocity.x < 0) velocity.x = std::max(velocity.x, -landMaxSpeed);
    position += velocity * timeDelta;

    switch (state) {
        case PlayerState::GROUNDED: currentAnimation = (std::fabs(velocity.x) > 0.1f) ? &runAnimation : &idleAnimation; break;
        case PlayerState::JUMPING: currentAnimation = (velocity.y < 0.0f) ? &jumpUpAnimation : &jumpDownAnimation; break;
        case PlayerState::WALL_KICK: currentAnimation = &jumpUpAnimation; break;
        case PlayerState::FALLING: currentAnimation = (velocity.y < 0.0f) ? &jumpUpAnimation : &jumpDownAnimation; break;
        case PlayerState::GRABBING: currentAnimation = &glideAnimation; break;
        case PlayerState::GLIDING: currentAnimation = &grabAnimation; break;
    }
}

void Player::draw() {
    if (playerDead) {
        if (!playerHide) {
            auto [origin, image, sound] = actuallyDead ? hurtAnimation.spriteForTime(animTime) : idleAnimation.spriteForTime(animTime);
            if (sound)
                sound->Play();
            game.drawSprite(position, image, origin, facingDirection == -1);
        }
        return;
    }

    auto [origin, image, sound] = currentAnimation->spriteForTime(animTime);
    if (sound)
        sound->Play();
    game.drawSprite(position, image, origin, facingDirection == -1);

    auto hitBoxPosition = game.worldToScreen(position - origin + hitbox.GetPosition());
    //DrawRectangleLines(hitBoxPosition.x, hitBoxPosition.y, hitbox.GetWidth(), hitbox.GetHeight(), RED);
}

void Player::load() {
    TraceLog(LOG_INFO, "Loading Player data.");

    const std::string playerFile = "Graphics/Player/player.json";
    auto jsonText = loadTextFile(playerFile);
    auto json = nlohmann::json::parse(jsonText);
    auto basePath = std::filesystem::path(playerFile).parent_path();

    landMaxSpeed = json["landMaxSpeed"].get<float>();
    landAcceleration = json["landAcceleration"].get<float>();
    landDeceleration = json["landDeceleration"].get<float>();
    landHardDeceleration = json["landHardDeceleration"].get<float>();
    airCorrectionAcceleration = json["airCorrectionAcceleration"].get<float>();

    jumpVelocity = json["jumpVelocity"].get<float>();
    jumpAccelerationTime = json["jumpAccelerationTime"].get<float>();
    wallKickVelocity = raylib::Vector2{ json["wallKickVelocity"]["x"].get<float>(), json["wallKickVelocity"]["y"].get<float>() };
    wallKickAccelerationTime = json["wallKickAccelerationTime"].get<float>();
    jumpBackPenalty = json["jumpBackPenalty"].get<float>();

    gravity = json["gravity"].get<float>();
    glidingGravity = json["glidingGravity"].get<float>();
    jumpSustainGravity = json["jumpSustainGravity"].get<float>();
    wallKickSustainGravity = raylib::Vector2{ json["wallKickSustainGravity"]["x"].get<float>(), json["wallKickSustainGravity"]["y"].get<float>() };
    jumpButtonActiveTime = json["jumpButtonActiveTime"].get<float>();
    hitbox = raylib::Rectangle{ json["hitbox"]["x"].get<float>(), json["hitbox"]["y"].get<float>(), json["hitbox"]["width"].get<float>(), json["hitbox"]["height"].get<float>() };

    cameraWindow = loadJsonRect(json["cameraWindow"]);
}

void Player::setPlayerDead(bool dead) {
    playerDead = true;
    actuallyDead = dead;
    if (!dead) {
        position = game.level.levelExitDoor.GetPosition();
        position.x += game.level.levelExitDoor.width / 2;
    }
}
