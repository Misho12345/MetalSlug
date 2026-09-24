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
        const Transform* cam = scene.try_get<Transform>(scene.camera());
        if (!cam) return;

        const ComponentPool<SpriteCollider>& collider_pool = scene.pool<SpriteCollider>();

        const vector<SpriteCollider>& colliders = collider_pool.components();
        const vector<entity_id>& owners = collider_pool.owners();

        assert(colliders.size() == owners.size());

        for (size_t i = 0; i < owners.size(); ++i)
        {
            const Transform* t = scene.try_get<Transform>(owners[i]);
            if (!t) continue;

            ui.draw_box(colliders[i].bounds(*t) - cam->position, HITBOX_COLOR);

            if (const SpriteRenderer* sr = scene.try_get<SpriteRenderer>(owners[i]))
            {
                aabb bounds = sr->screen_bounds(*t, cam->position);
                FrameMask mask = App::priv_ctx().sprite_data_registry.mask(sr->info(), sr->frame());

                for (int y = bounds.min.y; y < bounds.max.y; y += t->scale.y)
                {
                    for (int x = bounds.min.x; x < bounds.max.x; x += t->scale.x)
                    {
                        glm::ivec2 coords{ x, y };
                        if (mask[(coords - bounds.min) / t->scale])
                            ui.draw_box({ coords, coords + t->scale }, PIXEL_COLOR, 1.0f);
                    }
                }
            }
        }
    }

    void DebugDraw::draw_tile_map() const
    {
        const DebugUI& ui = App::priv_ctx().debug_ui;
        const TileType* t = App::priv_ctx().tile_map.data();
        const glm::ivec2 size = App::priv_ctx().tile_map.size();

        for (int y = 0; y < size.y; ++y)
        {
            for (int x = 0; x < size.x; ++x, ++t)
            {
                if (*t == TileType::Air) continue;

                const glm::ivec2 coords{ x, y };
                ui.draw_box(aabb{ coords, coords + 1 } * Target::TILE_SIZE, tile_color(*t));
            }
        }
    }
}
#endif
