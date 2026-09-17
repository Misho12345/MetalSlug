#include "mse/pch.hpp"
#include "animation_system.hpp"

#include "mse/components.hpp"
#include "mse/app.hpp"

#define ERR_MSG(MSG) "cannot load sprites, invalid json format - " MSG ";\n"

namespace mse
{
    bool AnimationSystem::init()
    {
        // allocate enough memory for all sprite animation data
        for (int i = 0; i < anim::sprite_count; ++i)
        {
            sprite_anim_datas_[i].resize(anim::sprite_anim_count(i));
        }

        /* JSON metadata format
         * {
         *   "atlases": [
         *     [
         *       {
         *         "sprite_id": <uint>,
         *         "anim_id": <uint>,
         *         "frame_count": <uint>,
         *         "x": <uint>,
         *         "y": <uint>,
         *         "w": <uint>,
         *         "h": <uint>
         *       }, { ... }, ...
         *     ], [...], ...
         *   ]
         * }
         */

        string s = FileIO::read("texture_packer/meta.json");

        if (s.empty())
        {
            printf(ERR_MSG("empty or nonexistent json file"));
            return false;
        }

        json j;

        try { j = json::parse(s.data()); }
        catch (const json::parse_error& e)
        {
            printf(ERR_MSG("%s"), e.what());
            return false;
        }

        if (!j.contains("atlases") || !j["atlases"].is_array())
        {
            printf(ERR_MSG("the json should contain \"atlases\": [...] ") "\n%s", j.dump().c_str());
            return false;
        }

        // "C++ is a strongly typed language, we shouldn't use auto"
        // - Abhishek
        for (const decltype(j.items().begin())& atlas : j["atlases"].items())
        {
            ++atlas_count_;

            if (!atlas.value().is_array())
            {
                printf(ERR_MSG("every member of \"atlases\": [...] should be an array") "\n%s", atlas.value().dump().c_str());
                return false;
            }

            for (const decltype(atlas.value().items().begin())& sprite : atlas.value().items())
            {
                const json& v = sprite.value();

                if (!v.contains("sprite_id") || !v["sprite_id"].is_number_unsigned() ||
                    !v.contains("anim_id") || !v["anim_id"].is_number_unsigned() ||
                    !v.contains("frame_count") || !v["frame_count"].is_number_unsigned() ||
                    !v.contains("x") || !v["x"].is_number_unsigned() ||
                    !v.contains("y") || !v["y"].is_number_unsigned() ||
                    !v.contains("w") || !v["w"].is_number_unsigned() ||
                    !v.contains("h") || !v["h"].is_number_unsigned())
                {
                    printf(ERR_MSG("invalid sprite animation data format") "\n%s", v.dump().c_str());
                    return false;
                }

                SpriteAnimationData& anim_data = sprite_anim_datas_
                        [v["sprite_id"].get<uint32_t>()]
                        [v["anim_id"].get<uint32_t>()];

                anim_data.offset      = { v["x"].get<uint32_t>(), v["y"].get<uint32_t>() };
                anim_data.image_size  = { v["w"].get<uint32_t>(), v["h"].get<uint32_t>() };
                anim_data.frame_count = v["frame_count"].get<uint32_t>();
                anim_data.atlas_idx   = std::stoi(atlas.key());
            }
        }

        return true;
    }

    void AnimationSystem::update(Scene& scene, const float dt) const
    {
        for (SpriteRenderer& sprite_renderer : scene.pool<SpriteRenderer>().components())
        {
            sprite_renderer.update(dt, anim_data(sprite_renderer.info()).frame_count);
        }
    }

    const SpriteAnimationData& AnimationSystem::anim_data(const anim::Info info) const
    {
        return sprite_anim_datas_[info.sprite_id][info.anim_id];
    }
}