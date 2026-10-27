#include "enemy.hpp"
#include "projectile.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        constexpr uint16_t CORPSE_TICKS = 24;
        constexpr uint16_t BLINK_TICKS  = 2;

        constexpr uint32_t KILL_POINTS = 100;
    }


    ivec2 feet(const Scene& scene, const entity_id e)
    {
        const aabb b = scene.get<Collider>(e).bounds(scene.get<Transform>(e));
        return { (b.min.x + b.max.x) / 2, b.max.y };
    }

    ivec2 to_player(const Scene& scene, const entity_id e)
    {
        return feet(scene, MetalSlug::user_ctx().player.legs()) - feet(scene, e);
    }

    void anchor(Scene& scene, const entity_id e, ivec2 origin)
    {
        Transform& tr  = scene.get<Transform>(e);
        Collider&  col = scene.get<Collider>(e);
        Sprite&    spr = scene.get<Sprite>(e);

        spr.flip_x = scene.get<Enemy>(e).facing > 0;
        if (spr.flip_x) origin.x = -origin.x;

        const ivec2 shift = feet(scene, e) + origin - tr.position;
        tr.position += shift;
        col.offset  -= shift;
    }

    void on_enemy_shot(Scene& scene, const entity_id self, const entity_id other, uint32_t)
    {
        Enemy& en = scene.get<Enemy>(self);

        // two bullets can land on the same tick, before its state machine gets to stop_colliding
        if (!en.shot) MetalSlug::user_ctx().player.add_score(KILL_POINTS);
        en.shot = true;

        const float vx = scene.get<Collider>(other).velocity.x;
        en.shot_dir    = (int8_t)(vx == 0.0f ? 0 : vx < 0.0f ? -1 : 1);

        if (scene.get<Projectile>(other).type == ProjectileType::Fireball) en.burning = true;
    }

    void stop_colliding(Scene& scene, const entity_id e)
    {
        Collider& col    = scene.get<Collider>(e);
        col.layer        = 0;
        col.target_layer = 0;
        col.velocity.x   = 0.0f;
    }

    void blink_out(Scene& scene, const entity_id e)
    {
        Enemy& en = scene.get<Enemy>(e);
        scene.get<Sprite>(e).hidden = (++en.corpse_ticks / BLINK_TICKS) % 2 != 0;

        if (en.corpse_ticks >= CORPSE_TICKS) scene.destroy_entity(e);
    }
}
