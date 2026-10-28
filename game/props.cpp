#include "props.hpp"

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
            float   parallax;
            int32_t nudge_x;
        };

        constexpr int32_t SKY_LAYER   = -200;
        constexpr int32_t FAR_LAYER   = -120; // behind bg.1, it shows where the white sky was cut out

        // under the characters, in front it would cover the aqua rebels, their idle is just a head above the water
        constexpr int32_t WATER_LAYER = -98;

        constexpr uint8_t WATER_TICKS = 6;
        constexpr uint8_t FALL_TICKS  = 4;

        constexpr float FAR = 0.1f;

        // camera x where the far props line up with their markers. At 0.1 the ship and the waterfall only slide
        // ~130 px across the whole village, at 0.25 they slid past the huts too fast to read as far away
        constexpr int32_t PARALLAX_REF_X = 2400;

        // indexed by the tile ids in props.tsx. The nudges are there because the water's first frame
        // is painted into bg.1 off the 8 px grid
        constexpr PropData PROPS[]
        {
            { Bg::WaterBack1, WATER_TICKS, WATER_LAYER, 1.0f, 4 },
            { Bg::WaterBack2, WATER_TICKS, WATER_LAYER, 1.0f, 0 },
            { Bg::WaterBack3, WATER_TICKS, WATER_LAYER, 1.0f, 0 },
            { Bg::WaterFront1, WATER_TICKS, WATER_LAYER, 1.0f, 4 },
            { Bg::WaterFront2, WATER_TICKS, WATER_LAYER, 1.0f, 1 },
            { Bg::WaterFront3, WATER_TICKS, WATER_LAYER, 1.0f, 3 },
            { Bg::WaterfallBack, FALL_TICKS, WATER_LAYER - 1, 1.0f, 0 },
            { Bg::WaterfallFront, FALL_TICKS, WATER_LAYER, 1.0f, 0 },
            { Bg::WaterfallPar, FALL_TICKS, FAR_LAYER, FAR, 0 },
            { Bg::Ship, FALL_TICKS, FAR_LAYER, FAR, 0 },
            { Bg::MetalFrame, 0, 3, 1.0f, 0 },
        };


        // 8 px wide and scaled to the screen, no point in a 304 px png of a vertical gradient
        void create_sky(Scene& scene)
        {
            const entity_id e = scene.create_entity();
            Transform&      tr = scene.set<Transform>(e, HALF_RESOLUTION_i);

            Sprite& s = scene.set<Sprite>(e, Bg::Sky);
            s.parallax_factor = { 0.0f, 0.0f };
            s.layer           = SKY_LAYER;

            tr.scale = { Target::RESOLUTION.x / s.size().x, 1 };
        }

        // marker is the bottom left cell and not the top left like the rock, the ship is 230 px tall
        // and its top would be above the map
        void create_prop(Scene& scene, const PropData& p, const ivec2 bottom_left)
        {
            const entity_id e = scene.create_entity();
            Transform&      tr = scene.set<Transform>(e);

            Sprite& s = scene.set<Sprite>(e, p.anim, p.ticks, should_loop::yes);
            s.layer           = p.layer;
            s.parallax_factor = { p.parallax, 1.0f };

            const ivec2 size = s.size();

            ivec2 top_left = bottom_left + ivec2{ p.nudge_x, -size.y };
            top_left.x -= (int32_t)((1.0f - p.parallax) * (float)PARALLAX_REF_X);

            tr.position = top_left + size / 2;
        }
    }


    void load_props(Scene& scene)
    {
        create_sky(scene);

        const Csv csv = FileIO::read<Csv>("assets/tile_maps/mission1_props.csv");
        const int32_t* d = csv.data.data();

        const ivec2 off = scene.get<Sprite>(scene.bg_entity).bounds(scene.get<Transform>(scene.bg_entity)).min;

        for (uint32_t y = 0; y < csv.height; ++y)
        {
            for (uint32_t x = 0; x < csv.width; ++x, ++d)
            {
                if (*d < 0) continue;

                assert((size_t)*d < std::size(PROPS) && "unknown prop");
                create_prop(scene, PROPS[*d], ivec2{ x, y + 1 } * TILE_SIZE_i + off);
            }
        }
    }
}
