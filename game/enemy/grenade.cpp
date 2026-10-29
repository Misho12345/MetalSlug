#include "grenade.hpp"
#include "explosive.hpp"
#include "layer.hpp"
#include "projectile.hpp"

namespace ms
{
    using namespace mse;
    using namespace mse::literals;

    namespace
    {
        enum class GrenadeState : uint8_t { Flying, COUNT };
        using GrenadeSM = StateMachine<GrenadeState>;

        constexpr uvec2 SIZE{ 10, 10 };

        constexpr float GRAVITY        = 500.0f;
        constexpr float MAX_FALL_SPEED = 1000.0f; // the default 300 would clamp the way down and miss the target

        // seconds, longer the further it goes, so far throws aren't flat and near ones don't go up like a rocket
        constexpr float   FLIGHT_TIME    = 0.6f;
        constexpr float   FLIGHT_TIME_PX = 1.0f / 400.0f;
        constexpr int32_t AIM_ERROR      = 16;

        constexpr uint8_t SPIN_TICKS = 3;

        // the asset has red and blue frames between the normal ones, a tint is close enough
        constexpr color_t RED  = 0xff5050_rgb;
        constexpr color_t BLUE = 0x509fbf_rgb;


        GrenadeState on_flying(const GrenadeSM::Ctx& c)
        {
            Sprite& s = c.scene.get<Sprite>(c.entity);

            switch (s.frame())
            {
                case 1: s.color = RED; break;
                case 3: s.color = BLUE; break;
                default: s.color = {}; break;
            }

            return c.scene.get<Explosive>(c.entity).exploded ? GrenadeState::COUNT : GrenadeState::Flying;
        }

        // where it landed
        void blow_up(const GrenadeSM::Ctx& c)
        {
            const Transform& tr = c.scene.get<Transform>(c.entity);
            explode(c.scene, c.entity, { tr.position.x, c.scene.get<Collider>(c.entity).bounds(tr).max.y });
        }

        constexpr GrenadeSM grenade_sm
        {
            GrenadeSM::Behaviour{ nullptr, on_flying, blow_up }, // Flying
        };
    }


    void register_grenade_machine() { App::ctx().state_machines.add(grenade_sm); }

    void launch_grenade(Scene& scene, const ivec2 from, const ivec2 target)
    {
        const entity_id g = scene.create_entity();

        scene.set<Transform>(g, from);
        scene.set<Sprite>(g, anim::Projectile::RebelGrenade, SPIN_TICKS, should_loop::yes).layer = 1;
        scene.set<Projectile>(g, ProjectileType::Grenade);
        scene.set<Explosive>(g);

        const vec2  d{ (float)(target.x - from.x + App::ctx().rng.range(-AIM_ERROR, AIM_ERROR)), (float)(target.y - from.y) };
        const float t = FLIGHT_TIME + glm::abs(d.x) * FLIGHT_TIME_PX;

        Collider& col = scene.set<Collider>(g, ivec2{ 0 }, ivec2(SIZE));

        col.gravity        = GRAVITY;
        col.max_fall_speed = MAX_FALL_SPEED;
        col.velocity       = { d.x / t, (d.y - 0.5f * GRAVITY * t * t) / t };

        col.layer        |= Layer::EnemyProjectile;
        col.target_layer |= Layer::Player | Layer::TileMap;
        col.callback      = on_explosive_hit;

        grenade_sm.start(scene, g, GrenadeState::Flying);
    }

    // the machine ends with the grenade, missiles have one of their own
    uint32_t live_grenades(Scene& scene) { return (uint32_t)scene.pool<State<GrenadeState>>().owners().size(); }
}
