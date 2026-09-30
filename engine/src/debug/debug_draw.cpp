#include "mse/pch.hpp"
#include "debug_draw.hpp"
#include "priv_ctx.hpp"
#include "mse/app.hpp"

#ifndef NDEBUG
namespace mse
{
    void DebugDraw::toggle() { active_ = !active_; }

    void DebugDraw::draw(const Scene& scene) const
    {
        if (!active_) return;

        draw_colliders(scene);
        draw_tile_map();
    }

    void DebugDraw::draw_colliders(const Scene& scene) const
    {
        const DebugUI& ui = App::priv_ctx().debug_ui;

        const ComponentPool<SpriteCollider>& collider_pool = scene.pool<SpriteCollider>();

        const vector<SpriteCollider>& colliders = collider_pool.components();
        const vector<entity_id>& owners = collider_pool.owners();

        assert(colliders.size() == owners.size());

        for (size_t i = 0; i < owners.size(); ++i)
        {
            const Transform& t = scene.get<Transform>(owners[i]);
            const SpriteRenderer& sr = scene.get<SpriteRenderer>(owners[i]);

            ui.draw_box(colliders[i].bounds(t), HITBOX_COLOR);

            const aabb bounds = sr.bounds(t);
            FrameMask mask = App::priv_ctx().sprite_data_registry.mask(sr.info(), sr.frame());

            for (int32_t y = bounds.min.y; y < bounds.max.y; )
            {
                int32_t y1 = y + static_cast<int32_t>(t.scale.y);

                for (int32_t x = bounds.min.x; x < bounds.max.x; )
                {
                    int32_t x1 = x + static_cast<int32_t>(t.scale.x);

                    const ivec2 coords{ x, y };
                    if (mask[(coords - bounds.min) / ivec2(t.scale)])
                        ui.draw_box({ coords, { x1, y1 } }, PIXEL_COLOR, 1.0f);

                    x = x1;
                }

                y = y1;
            }
        }
    }

    void DebugDraw::draw_tile_map() const
    {
        const DebugUI& ui = App::priv_ctx().debug_ui;
        const TileType* t = App::priv_ctx().tile_map.data();
        const ivec2 size = App::priv_ctx().tile_map.size();

        for (ivec2 c{}; c.y < size.y; ++c.y)
        {
            for (c.x = 0; c.x < size.x; ++c.x, ++t)
            {
                if (*t == TileType::Air) continue;
                ui.draw_box(aabb{ c, c + 1 } * Target::TILE_SIZE, tile_color(*t));
            }
        }
    }
}
#endif
