#pragma once
#include "mse/mse.hpp"
#include "anim_data.hpp"
#include "weapons.hpp"

// components and types that everything uses
namespace ms
{
    enum class BodyState : uint8_t
    {
        Ready,
        Shooting,
        Transition, // turning, crouching down, standing up, raising/lowering the gun
        Dead,
        // Melee, Throwing,
        COUNT
    };

    enum class LegsState : uint8_t
    {
        Stand, Walk, Air, Crouch,
        COUNT
    };

    using LegsSM = StateMachine<LegsState>;
    using BodySM = StateMachine<BodyState>;

    enum class Aim : uint8_t { Forward, Up, Down };

    // the legs' collider, every sprite origin is relative to it
    // could've made it extracted at runtime, but it's completely unnecessary
    constexpr uvec2 LEGS_SIZE{ 21, 16 };

    constexpr uint8_t  START_LIVES        = 3;
    constexpr uint16_t DEAD_TICKS         = 150; // the death animation and lying there for a bit
    constexpr uint16_t INVULNERABLE_TICKS = 90; // after respawning


    // on the legs, rewritten by Player::update every tick
    struct Intent
    {
        enum Bits : uint8_t
        {
            Jump = 1 << 0,
            Fire = 1 << 1,
            // Throw = 1 << 2,
            // Drink = 1 << 3,
        };

        uint8_t down{};
        uint8_t just_pressed{};

        glm::i8vec2 dir{ 0 };
    };

    struct LegsData final
    {
        entity_id body;
        ivec2     sprite_offset{ 0 };
        int8_t    facing{ 1 };
    };

    struct BodyData final
    {
        entity_id legs;

        PoseAnim requested{ Body::COUNT };

        Pose transition{ Pose::COUNT };
        Pose last_pose{ Pose::COUNT };
        Aim  aim{};

        Pose death{ Pose::COUNT }; // what killed him, COUNT while alive, set by the colliders' callback

        uint8_t  lives = START_LIVES;
        uint16_t invulnerable{}; // ticks left

        WeaponType weapon{};
        uint32_t ammo = 0; // 0 for pistol = inf

        bool restart{ false };
        bool was_up{ false };
        bool fire_queued{ false };

        uint8_t burst_left{};
        uint8_t burst_shot{};
    };

    struct WeaponUpgrade final
    {
        WeaponType type{};
        bool claimed{ false };
    };

    inline entity_id legs_of(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).legs; }
    inline entity_id body_of(const LegsSM::Ctx& c) { return c.scene.get<LegsData>(c.entity).body; }
}
