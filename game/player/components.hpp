#pragma once
#include "mse/mse.hpp"
#include "anim_data.hpp"
#include "weapons.hpp"

// components and types that everything uses
namespace player
{
    enum class BodyState : uint8_t
    {
        Ready,
        Shooting,
        Transition, // turning, crouching down, standing up, raising/lowering the gun
        // Melee, Throwing, Dead,
        Count
    };

    enum class LegsState : uint8_t
    {
        Stand, Walk, Air, Crouch,
        Count
    };

    using LegsSM = StateMachine<LegsState>;
    using BodySM = StateMachine<BodyState>;

    enum class Aim : uint8_t { Forward, Up, Down };

    // the legs' collider, every sprite origin is relative to it
    // could've made it extracted at runtime, but it's completely unnecessary
    constexpr uvec2 LEGS_SIZE{ 21, 16 };


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

        PoseAnim requested{ Body::_COUNT };

        Pose transition{ Pose::Count };
        Pose last_pose{ Pose::Count };
        Aim  aim{};

        WeaponType weapon{};

        bool restart{ false };
        bool was_up{ false };
        bool fire_queued{ false };

        uint8_t burst_left{};
        uint8_t burst_shot{};
    };


    inline entity_id legs_of(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).legs; }
    inline entity_id body_of(const LegsSM::Ctx& c) { return c.scene.get<LegsData>(c.entity).body; }
}
