#include "aqua_rebel.hpp"
#include "missile.hpp"
#include "layer.hpp"
#include "spawn.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        using AquaAnim = anim::enemy::AquaRebel;
        using AquaSM   = StateMachine<AquaRebelState>;

        // all of it at the top of the jump, the mask decides what got hit, in the water that's just the head
        constexpr uvec2 BOX{ 28, 52 };
        constexpr float GRAVITY = 900.0f;

        constexpr int32_t SIGHT    = 160; // px on x, feet to feet
        constexpr int32_t DEADZONE = 8;

        // ticks, only counted down while the player's in sight
        constexpr int32_t NOTICE_MIN = 30; // the first jump after he shows up
        constexpr int32_t NOTICE_MAX = 90;
        constexpr int32_t RELOAD_MIN = 180;
        constexpr int32_t RELOAD_MAX = 300;

        // where the missile leaves from, from the feet facing left. The bottom of the swing rather than overhead,
        // from overhead even the smallest jump would send it over a player standing at the water's level
        constexpr ivec2    RELEASE{ -12, -24 };
        constexpr uint32_t RELEASE_FRAME = 1;
        constexpr uint32_t DIVE_FRAME    = 2; // curled up from here, it drops

        constexpr int32_t JUMP_MIN = 16; // it still has to come out of the water with the player down low
        constexpr int32_t JUMP_MAX = 120;


        struct AquaData final
        {
            int32_t  water{};  // y of the surface
            ivec2    target{}; // the player's chest when it jumped
            uint16_t cooldown{};
            bool     thrown{ false };
        };

        constexpr EnemyAnimData data(const AquaAnim a)
        {
            switch (a)
            {
                case AquaAnim::Idle: return { 8, { -1, -6 }, true };
                case AquaAnim::JumpingOut: return { 3, { 6, -27 } };
                case AquaAnim::ThrowingMissile: return { 4, { 6, -25 } };
                case AquaAnim::Submerging: return { 5, { 1, -9 } };
                case AquaAnim::Death: return { 4, { 8, -18 } };

                default: assert(!"not implemented");
            }

            return {};
        }


        AquaData& aqua(const AquaSM::Ctx& c) { return c.scene.get<AquaData>(c.entity); }
        Enemy&    enemy(const AquaSM::Ctx& c) { return c.scene.get<Enemy>(c.entity); }
        Random&   rng() { return App::ctx().rng; }

        void play(const AquaSM::Ctx& c, const AquaAnim a) { play_anchored(c.scene, c.entity, a, data(a)); }

        ivec2 chest(const Scene& scene)
        {
            const entity_id body = MetalSlug::user_ctx().player.body();
            const aabb      b    = scene.get<Collider>(body).bounds(scene.get<Transform>(body));
            return (b.min + b.max) / 2;
        }

        // falling through the surface puts it back on it
        bool back_in_water(const AquaSM::Ctx& c)
        {
            Collider&     col   = c.scene.get<Collider>(c.entity);
            const int32_t below = feet(c.scene, c.entity).y - aqua(c).water;

            if (col.velocity.y <= 0.0f || below < 0) return false;

            c.scene.get<Transform>(c.entity).position.y -= below;
            col.velocity = vec2{ 0.0f };
            col.gravity  = 0.0f;
            return true;
        }


        void idle_enter(const AquaSM::Ctx& c)
        {
            c.scene.get<Collider>(c.entity).enabled = true;
            play(c, AquaAnim::Idle);
        }

        AquaRebelState on_idle(const AquaSM::Ctx& c)
        {
            if (enemy(c).shot) return AquaRebelState::Dead;

            const ivec2 to = to_player(c.scene, c.entity);
            if (!MetalSlug::user_ctx().player.alive(c.scene) || glm::abs(to.x) > SIGHT) return AquaRebelState::Idle;

            Enemy& en = enemy(c);
            if (glm::abs(to.x) > DEADZONE && (to.x > 0) != (en.facing > 0))
            {
                en.facing = (int8_t)-en.facing;
                anchor(c.scene, c.entity, data(AquaAnim::Idle).origin);
            }

            AquaData& d = aqua(c);
            if (d.cooldown == 0) return AquaRebelState::Jump;

            --d.cooldown;
            return AquaRebelState::Idle;
        }


        void jump_enter(const AquaSM::Ctx& c)
        {
            AquaData& d = aqua(c);
            d.cooldown  = (uint16_t)rng().range(RELOAD_MIN, RELOAD_MAX);
            d.target    = chest(c.scene);
            d.thrown    = false;

            // so the release point tops out level with his chest, the missile drifts the rest when it's clamped
            const int32_t h = glm::clamp(d.water - (d.target.y - RELEASE.y), JUMP_MIN, JUMP_MAX);

            Collider& col  = c.scene.get<Collider>(c.entity);
            col.gravity    = GRAVITY;
            col.velocity.y = -glm::sqrt(2.0f * GRAVITY * (float)h);

            play(c, AquaAnim::JumpingOut);
        }

        AquaRebelState on_jump(const AquaSM::Ctx& c)
        {
            if (enemy(c).shot) return AquaRebelState::Dead;
            return c.scene.get<Collider>(c.entity).velocity.y >= 0.0f ? AquaRebelState::Throw : AquaRebelState::Jump;
        }


        void throw_enter(const AquaSM::Ctx& c)
        {
            Collider& col = c.scene.get<Collider>(c.entity);
            col.gravity   = 0.0f;
            col.velocity  = vec2{ 0.0f };

            play(c, AquaAnim::ThrowingMissile);
        }

        AquaRebelState on_throw(const AquaSM::Ctx& c)
        {
            if (enemy(c).shot) return AquaRebelState::Dead;

            AquaData&      d     = aqua(c);
            const uint32_t frame = c.scene.get<Sprite>(c.entity).frame();

            if (!d.thrown && frame >= RELEASE_FRAME)
            {
                const int8_t facing = enemy(c).facing;
                const ivec2  from   = feet(c.scene, c.entity) + ivec2{ facing < 0 ? RELEASE.x : -RELEASE.x, RELEASE.y };

                // creates an entity, no component references across this
                launch_missile(c.scene, from, facing, d.target);
                d.thrown = true;
            }

            if (frame >= DIVE_FRAME) c.scene.get<Collider>(c.entity).gravity = GRAVITY;
            return back_in_water(c) ? AquaRebelState::Submerge : AquaRebelState::Throw;
        }


        // just the splash, nothing to shoot at
        void submerge_enter(const AquaSM::Ctx& c)
        {
            c.scene.get<Collider>(c.entity).enabled = false;
            play(c, AquaAnim::Submerging);
        }

        // in the real game it technically disappears once it's jumped, but with this few enemy types
        // thats boring, so it goes back to idle until the player actually kills it
        AquaRebelState on_submerge(const AquaSM::Ctx& c)
        {
            return c.scene.get<Sprite>(c.entity).anim_done() ? AquaRebelState::Idle : AquaRebelState::Submerge;
        }


        void dead_enter(const AquaSM::Ctx& c)
        {
            stop_colliding(c.scene, c.entity);

            // shot out of the air, it still comes down
            if (feet(c.scene, c.entity).y < aqua(c).water) c.scene.get<Collider>(c.entity).gravity = GRAVITY;

            play(c, AquaAnim::Death);
            MetalSlug::user_ctx().sounds.rebel_death();
        }

        AquaRebelState on_dead(const AquaSM::Ctx& c)
        {
            back_in_water(c);
            if (c.scene.get<Sprite>(c.entity).anim_done()) blink_out(c.scene, c.entity);

            return AquaRebelState::Dead;
        }


        using B = AquaSM::Behaviour;
        constexpr AquaSM aqua_sm
        {
            B{ idle_enter, on_idle, nullptr },         // Idle
            B{ jump_enter, on_jump, nullptr },         // Jump
            B{ throw_enter, on_throw, nullptr },       // Throw
            B{ submerge_enter, on_submerge, nullptr }, // Submerge
            B{ dead_enter, on_dead, nullptr },         // Dead
        };
    }


    void AquaRebelManager::register_machine() const { App::ctx().state_machines.add(aqua_sm); }

    entity_id AquaRebelManager::spawn(Scene& scene, const ivec2 pos) const
    {
        const entity_id e = spawn_standing(scene, pos, AquaAnim::Idle);

        scene.set<Enemy>(e);

        AquaData& d = scene.set<AquaData>(e);
        d.water     = pos.y;
        d.cooldown  = (uint16_t)rng().range(NOTICE_MIN, NOTICE_MAX);

        const ivec2 box_center = pos - ivec2{ 0, (int32_t)BOX.y / 2 };
        Collider&   col        = scene.set<Collider>(e, box_center - scene.get<Transform>(e).position, ivec2(BOX));

        // floats, the jump turns gravity on, and the tiles under the water aren't its business
        col.gravity               = 0.0f;
        col.collide_with_tile_map = false;
        col.layer                 |= Layer::Enemy;
        col.target_layer          |= Layer::PlayerProjectile;
        col.callback              = on_enemy_shot;

        aqua_sm.start(scene, e, AquaRebelState::Idle);
        return e;
    }
}
