#include "mse/pch.hpp"
#include "debug_draw.hpp"
#include "priv_ctx.hpp"
#include "mse/app.hpp"

#ifndef NDEBUG
namespace mse
{
    using namespace literals;

    namespace
    {
        constexpr color TILE_EMPTY_COLOR{ 0x888888_rgb };
        constexpr color TILE_BLOCK_COLOR{ 0xff0000_rgb };
        constexpr color TILE_PASS_COLOR{ 0x00A2E8_rgb };
        constexpr color HITBOX_COLOR{ 0x00ff00_rgb };
        constexpr color PIXEL_COLOR{ 0x00c8c8_rgb };
    }

    void DebugDraw::toggle() { active_ = !active_; }

    void DebugDraw::draw(const Scene& scene) const
    {
        if (!active_) return;

        draw_colliders(scene);
        draw_tile_map(scene);
    }

    void DebugDraw::draw_colliders(const Scene& scene) const
    {
        const DebugUI& ui = App::priv_ctx().debug_ui;

        const ComponentPool<Collider>& collider_pool = scene.pool<Collider>();

        const vector<Collider>& colliders = collider_pool.components();
        const vector<entity_id>& owners = collider_pool.owners();

        assert(colliders.size() == owners.size());

        for (size_t i = 0; i < owners.size(); ++i)
        {
            const Transform& t = scene.get<Transform>(owners[i]);
            const Sprite& s = scene.get<Sprite>(owners[i]);

            ui.draw_box(colliders[i].bounds(t), HITBOX_COLOR, 5.0f);

            const aabb bounds = s.bounds(t);
            FrameMask mask = App::priv_ctx().sprite_data_registry.mask(s.info(), s.frame(), s.flip_x);

            for (int32_t y = bounds.min.y; y < bounds.max.y; )
            {
                int32_t y1 = y + (int32_t)t.scale.y;

                for (int32_t x = bounds.min.x; x < bounds.max.x; )
                {
                    int32_t x1 = x + (int32_t)t.scale.x;

                    const ivec2 coords{ x, y };
                    if (mask[(coords - bounds.min) / ivec2(t.scale)])
                        ui.draw_box({ coords, { x1, y1 } }, PIXEL_COLOR, 1.0f);

                    x = x1;
                }

                y = y1;
            }
        }
    }

    void DebugDraw::draw_tile_map(const Scene& scene) const
    {
        const Transform& bg_tr = scene.get<Transform>(scene.bg_entity);
        const Sprite& bg_sprite = scene.get<Sprite>(scene.bg_entity);

        const ivec2 origin = bg_tr.position - ivec2(bg_sr.size() / 2);

        const DebugUI& ui = App::priv_ctx().debug_ui;

        const TileMap& tm = App::priv_ctx().tile_map;
        const TileType* t = tm.data();
        const ivec2 size = tm.size();

        // i know it's not the most optimal way and culling should be done here, and not in draw box and line
        // but it doesn't matter really, this is for debug purposes

        for (int y = 0; y <= size.y; ++y)
        {
            ui.draw_line(
                origin + TILE_SIZE_i * ivec2{ 0, y },
                origin + TILE_SIZE_i * ivec2{ size.x, y },
                TILE_EMPTY_COLOR, 1.0f);
        }

        for (int x = 0; x < size.x; ++x)
        {
            ui.draw_line(
                origin + TILE_SIZE_i * ivec2{ x, 0 },
                origin + TILE_SIZE_i * ivec2{ x, size.y },
                TILE_EMPTY_COLOR, 1.0f);
        }

        for (ivec2 c{}; c.y < size.y; ++c.y)
        {
            for (c.x = 0; c.x < size.x; ++c.x, ++t)
            {
                const ivec2 tl = origin + c * TILE_SIZE_i;
                const ivec2 tr = tl + ivec2{ TILE_SIZE_i.x, 0 };
                const ivec2 bl = tl + ivec2{ 0, TILE_SIZE_i.y };
                const ivec2 br = tl + TILE_SIZE_i;

                switch (*t)
                {
                    case TileType::Air: continue;

                    case TileType::Floor:
                        ui.draw_box(aabb{ tl, br }, TILE_BLOCK_COLOR);
                        ui.draw_line(tl, br, TILE_BLOCK_COLOR);
                        ui.draw_line(tr, bl, TILE_BLOCK_COLOR);
                        break;

                    case TileType::Platform:
                        ui.draw_line(tl, tr, TILE_BLOCK_COLOR);
                        ui.draw_line(tl, bl, TILE_PASS_COLOR);
                        ui.draw_line(bl, br, TILE_PASS_COLOR);
                        ui.draw_line(tr, br, TILE_PASS_COLOR);
                        break;

                    case TileType::SlopeL:
                        ui.draw_line(tl, bl, TILE_BLOCK_COLOR);
                        ui.draw_line(tl, br, TILE_BLOCK_COLOR);
                        ui.draw_line(bl, br, TILE_BLOCK_COLOR);
                        break;

                    case TileType::SlopeR:
                        ui.draw_line(bl, tr, TILE_BLOCK_COLOR);
                        ui.draw_line(bl, br, TILE_BLOCK_COLOR);
                        ui.draw_line(tr, br, TILE_BLOCK_COLOR);
                        break;
                }

            }
        }
    }
}
#endif
