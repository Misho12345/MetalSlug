#include "missile.hpp"
#include "explosive.hpp"
#include "layer.hpp"
#include "projectile.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        enum class MissileState : uint8_t { Flying, COUNT };
        using MissileSM = StateMachine<MissileState>;

        // the body without the flame, the nose is at the left of the art
        constexpr uvec2   BOX{ 32, 10 };
        constexpr int32_t BOX_X = -13;

        constexpr float    SPEED     = 150.0f;
        constexpr float    MAX_DRIFT = 60.0f; // px/s on y, past this it'd look like it's falling
        constexpr uint16_t LIFE      = 240;   // ticks, well off screen by then

        constexpr uint8_t FLAME_TICKS = 3;


        MissileState on_flying(const MissileSM::Ctx& c)
        {
            if (c.scene.get<Explosive>(c.entity).exploded || c.state.ticks >= LIFE) return MissileState::COUNT;
            return MissileState::Flying;
        }

        void end(const MissileSM::Ctx& c)
        {
            if (!c.scene.get<Explosive>(c.entity).exploded)
            {
                c.scene.destroy_entity(c.entity);
                return;
            }

            const aabb b = c.scene.get<Collider>(c.entity).bounds(c.scene.get<Transform>(c.entity));

            // at the nose. The flip and not the velocity, a wall zeroes that
            explode(c.scene, c.entity, { c.scene.get<Sprite>(c.entity).flip_x ? b.max.x : b.min.x, b.max.y });
        }

        constexpr MissileSM missile_sm
        {
            MissileSM::Behaviour{ nullptr, on_flying, end }, // Flying
        };
    }


    void register_missile_machine() { App::ctx().state_machines.add(missile_sm); }

    void launch_missile(Scene& scene, const ivec2 from, const int8_t dir, const ivec2 target)
    {
        const entity_id m = scene.create_entity();

        scene.set<Transform>(m, from);
        scene.set<Projectile>(m, ProjectileType::Missile);
        scene.set<Explosive>(m);

        Sprite& spr = scene.set<Sprite>(m, anim::Projectile::AquaRebelMissile, FLAME_TICKS, should_loop::yes);
        spr.flip_x  = dir > 0;
        spr.layer   = 3;

        Collider& col = scene.set<Collider>(m, ivec2{ dir < 0 ? BOX_X : -BOX_X, 0 }, ivec2(BOX));

        // the jump got it to the right height already when it could, this is what's left over
        const float dist  = glm::max((float)glm::abs(target.x - from.x), 1.0f);
        const float drift = (float)(target.y - from.y) * SPEED / dist;

        col.gravity  = 0.0f;
        col.velocity = { (float)dir * SPEED, glm::clamp(drift, -MAX_DRIFT, MAX_DRIFT) };

        col.layer        |= Layer::EnemyProjectile;
        col.target_layer |= Layer::Player | Layer::TileMap;
        col.callback      = on_explosive_hit;

        missile_sm.start(scene, m, MissileState::Flying);
    }
}
