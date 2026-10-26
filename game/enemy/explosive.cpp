#include "explosive.hpp"
#include "spawn.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        constexpr uint8_t EXPLOSION_TICKS = 2;
        constexpr int32_t EXPLOSION_LAYER = 2;
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
        const entity_id boom = MetalSlug::user_ctx().vfx.create(
            scene, {}, anim::Vfx::SmallExplosion, EXPLOSION_TICKS, EXPLOSION_LAYER);

        align(scene, boom, at);
        scene.destroy_entity(e);
    }
}
