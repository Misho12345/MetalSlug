#pragma once
#include "player_anim.hpp"
#include "mse/mse.hpp"


enum class BodyState : uint8_t
{
    Ready, Shooting,
    Transition, // turning, crouching down, standing up, raising / lowering the gun: no shooting meanwhile
    // Melee, Throwing, Dead,
    Count
};

enum class LegsState : uint8_t
{
    Stand, Walk, Air, Crouch,
    Count
};


enum class WeaponType : uint8_t
{
    Pistol,
    RifleSpray,
    RifleFireball,
    Count
};


namespace player
{
    using LegsSM = StateMachine<LegsState>;
    using BodySM = StateMachine<BodyState>;

    class Player
    {
        static constexpr size_t BULLET_POOL_SIZE = 32;

        static constexpr int32_t SHOOT_OFFSET = 10;

        // ----- Move -----

        static constexpr float JUMP_HEIGHT    = 56.0f;
        static constexpr float JUMP_APEX_TIME = 0.36f; // from takeoff to the highest point

        static constexpr float JUMP_GRAVITY  = 2.0f * JUMP_HEIGHT / (JUMP_APEX_TIME * JUMP_APEX_TIME);
        static constexpr float JUMP_VELOCITY = 2.0f * JUMP_HEIGHT / JUMP_APEX_TIME;

        static constexpr float WALK_SPEED  = 100.0f;
        static constexpr float CRAWL_SPEED = 50.0f;


        // ----- Colliders -----

        static constexpr uvec2 LEGS_SIZE{ 21, 16 };

        // legs collider for tilemap for when hidden
        static constexpr uint32_t LEGS_SMALL_HEIGHT = 4;

        // keeps the bottom of the small box where the bottom of the full one is
        static constexpr int32_t LEGS_SMALL_OFFSET = (int32_t)(LEGS_SIZE.y / 2) - (int32_t)(LEGS_SMALL_HEIGHT / 2);

        static constexpr ivec2 CAMERA_OFFSET{ 0, -22 };


    public:
        explicit Player(Scene& scene);

        static void register_machines();

        void update(Scene& scene) const;
        void late_update(Scene& scene) const;

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

    private:
        enum class Aim : uint8_t { Forward, Up, Down };


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

            uint32_t next_bullet{};
        };


        struct PlayerBullet final
        {
            WeaponType type;
        };




        struct BodyStateManager;
        struct LegsStateManager;


        void update_bullets(Scene& scene) const;

        static void stop_bullet(Scene& scene, entity_id bullet, PlayerBullet type);

        static void      shoot(Scene& scene, entity_id body, Aim aim, int8_t dir);
        static entity_id take_bullet(Scene& scene, BodyData& data);

        static entity_id legs_of(const BodySM::Ctx& c);
        static entity_id body_of(const LegsSM::Ctx& c);

        static void begin_transition(Scene& scene, entity_id body, Pose pose);
        static ivec2 sprite_center(ivec2 box_center, ivec2 origin, ivec2 size, bool flip);

        static Pose pick_pose(
            BodyState               body,
            const State<LegsState>& legs,
            Intent                  in,
            bool                    side_jump,
            const BodyData&         bd);


        static const BodySM& body_sm();
        static const LegsSM& legs_sm();

        entity_id legs_;
        entity_id body_;
    };
}
