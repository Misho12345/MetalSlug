#include "explosive.hpp"
#include "spawn.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        constexpr uint8_t EXPLOSION_TICKS = 2;
        constexpr int32_t EXPLOSION_LAYER = 3;
    }


    void on_explosive_hit(Scene& scene, const entity_id self, entity_id, uint32_t)
    {
        // the dispatch doesn't check enabled, hitting the tile map and the player in one tick would explode it twice
        Collider& col = scene.get<Collider>(self);
        if (!col.enabled) return;

        col.enabled = false;
        scene.get<Sprite>(self).hidden = true;
        scene.get<Explosive>(self).exploded = true;
    }

    void explode(Scene& scene, const entity_id e, const ivec2 at)
    {
        App::ctx().particles.emit(anim::Vfx::SmallExplosion, {
            .position  = standing_on(anim::Vfx::SmallExplosion, at),
            .frame_dur = EXPLOSION_TICKS,
            .layer     = EXPLOSION_LAYER
        });

        scene.destroy_entity(e);

        MetalSlug::user_ctx().sounds.explosion();
    }
}
