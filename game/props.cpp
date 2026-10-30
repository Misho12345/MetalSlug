#include "props.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;
    using anim::Bg;

    namespace
    {
        struct PropData final
        {
            Bg      anim;
            uint8_t ticks; // per frame
            int32_t layer;
            int32_t nudge_x;
        };

        constexpr int32_t FAR_LAYER  = -120; // behind bg.1, it shows where the white sky was cut out
        constexpr int32_t BACK_LAYER = -98;

        // the rock at trees-2's jagged right end, the arch the ship sits in past the village is this colour,
        // and the last few px of the level show it
        constexpr color_t BACKDROP{ 74, 73, 49 };

        // over the player and anyone wading, under the aqua rebels and explosions on 3
        constexpr int32_t FRONT_WATER_LAYER = 2;
        constexpr int32_t FOREGROUND_LAYER  = 4;

        constexpr uint8_t WATER_TICKS = 6;
        constexpr uint8_t FALL_TICKS  = 4;

        // the far layer's Parallax Factor in Tiled, and the map's Parallax Origin (map px). Tiled draws a layer
        // (view center - origin) * (1 - factor) away from where it is, so with the same numbers here the far layer
        // looks in the editor like it does in the game.
        // 0.75 is from a longplay, trees-2 moves 3 px for every 4 the camera does. The origin is picked so the
        // spot it was measured at lands on a whole tile
        constexpr float   FAR_PARALLAX      = 0.75f;
        constexpr int32_t PARALLAX_ORIGIN_X = 2568;

        // indexed by the tile ids in props.tsx. The nudges are there because the water's first frame
        // is painted into bg.1 off the 8 px grid
        constexpr PropData PROPS[]
        {
            { Bg::WaterBack1, WATER_TICKS, BACK_LAYER, 4 },
            { Bg::WaterBack2, WATER_TICKS, BACK_LAYER, 0 },
            { Bg::WaterBack3, WATER_TICKS, BACK_LAYER, 0 },
            { Bg::WaterFront1, WATER_TICKS, FRONT_WATER_LAYER, 4 },
            { Bg::WaterFront2, WATER_TICKS, FRONT_WATER_LAYER, 1 },
            { Bg::WaterFront3, WATER_TICKS, FRONT_WATER_LAYER, 3 },
            { Bg::WaterfallBack, FALL_TICKS, BACK_LAYER - 1, 4 },
            { Bg::WaterfallFront, FALL_TICKS, BACK_LAYER + 1, 1 }, // its own water covers the back water strips' ends
            { Bg::BossWaterfall, FALL_TICKS, FAR_LAYER + 1, 0 }, // its palm covers the ship's empty left edge
            { Bg::Ship, FALL_TICKS, FAR_LAYER, 0 },
            { Bg::MetalFrame, 0, FOREGROUND_LAYER, 5 }, // right leg on the hull opening's right edge
            {},                                         // FAR_FROM
            {},                                         // FAR_TO
            { Bg::Trees1, 0, FAR_LAYER, 0 },
            { Bg::Trees2, 0, FAR_LAYER, 0 },
        };

        // markers on the props layer, not sprites. The far layer is only drawn while the screen overlaps
        // the stretch between them, so it can't show through cut out sky anywhere else
        constexpr int32_t FAR_FROM = 11;
        constexpr int32_t FAR_TO   = 12;

        struct FarProp final
        {
            int32_t from{ INT32_MIN };
            int32_t to{ INT32_MAX };
        };


        // marker is the bottom left cell and not the top left like the rock, the ship is 230 px tall
        // and its top would be above the map. Tiled draws tiles bigger than the grid the same way
        entity_id create_prop(Scene& scene, const PropData& p, const ivec2 bottom_left, const float parallax)
        {
            const entity_id e = scene.create_entity();
            Transform&      tr = scene.set<Transform>(e);

            Sprite& s = scene.set<Sprite>(e, p.anim, p.ticks, should_loop::yes);
            s.layer           = p.layer;
            s.parallax_factor = { parallax, 1.0f };

            const ivec2 size = s.size();

            // the renderer only subtracts camera * parallax, the rest of Tiled's offset doesn't depend on the camera
            ivec2 top_left = bottom_left + ivec2{ p.nudge_x, -size.y };
            top_left.x -= (int32_t)((float)(PARALLAX_ORIGIN_X - HALF_RESOLUTION_i.x) * (1.0f - parallax));

            tr.position = top_left + size / 2;
            return e;
        }

        // calls f(prop id, bottom left of its cell) for every marker
        template <typename F>
        void for_each_marker(const Scene& scene, const char* path, F&& f)
        {
            const Csv csv = FileIO::read<Csv>(path);
            const int32_t* d = csv.data.data();

            const ivec2 off = scene.get<Sprite>(scene.bg_entity).bounds(scene.get<Transform>(scene.bg_entity)).min;

            for (uint32_t y = 0; y < csv.height; ++y)
            {
                for (uint32_t x = 0; x < csv.width; ++x, ++d)
                {
                    if (*d >= 0) f(*d, ivec2{ x, y + 1 } * TILE_SIZE_i + off);
                }
            }
        }

        const PropData& prop(const int32_t id)
        {
            assert((size_t)id < std::size(PROPS) && "unknown prop");
            return PROPS[id];
        }
    }


    void load_props(Scene& scene)
    {
        scene.backdrop = BACKDROP;

        FarProp window;

        for_each_marker(scene, "assets/tile_maps/mission1_props.csv", [&](const int32_t id, const ivec2 at)
        {
            if (id == FAR_FROM) window.from = at.x;
            else if (id == FAR_TO) window.to = at.x + TILE_SIZE_i.x;
            else
            {
                const PropData& p = prop(id);
                const entity_id e = create_prop(scene, p, at, 1.0f);

                // the boss's base is painted into its right end, so the arena goes where it is
                if (p.anim == Bg::WaterfallFront)
                {
                    const Transform& tr = scene.get<Transform>(e);
                    MetalSlug::user_ctx().boss.create(scene, scene.get<Sprite>(e).bounds(tr).min);
                }
            }
        });

        for_each_marker(scene, "assets/tile_maps/mission1_far.csv", [&](const int32_t id, const ivec2 at)
        {
            scene.set<FarProp>(create_prop(scene, prop(id), at, FAR_PARALLAX), window);
        });
    }

    void update_props(Scene& scene)
    {
        const int32_t left = scene.camera_pos_screen().x;

        for (const entity_id e : scene.pool<FarProp>().owners())
        {
            const FarProp& w = scene.get<FarProp>(e);
            scene.get<Sprite>(e).hidden = left + RESOLUTION_i.x <= w.from || left >= w.to;
        }
    }
}
