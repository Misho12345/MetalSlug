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
            if (const Transform* t = scene.try_get<Transform>(owners[i]))
            {
                ui.draw_box(colliders[i].bounds(*t), COLLIDER_COLOR);
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
