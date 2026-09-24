#include "mse/pch.hpp"
#include "sprite_data_registry.hpp"

#define JSON_FMT_ERR_MSG(MSG) "cannot load sprites, invalid json format - " MSG ";\n"

namespace mse
{
    SpriteDataRegistry::SpriteDataRegistry() : anim_data_(anim::total_anim_count)
    {
        mask_offsets_.reserve(anim::total_frame_count);
    }


    size_t FrameMask::range(const glm::ivec2 coords, const uint32_t max_size) const {
        // may need the chunk to start from a prev row
        assert(/*coords.x >= 0 &&*/ coords.x < size.x &&
               coords.y >= 0 && coords.y < size.y);

        // does not leak to the next row
        assert(static_cast<int>(max_size) + coords.x < size.x);

        const size_t idx = coords.x + coords.y * size.x;
        assert(idx % 8 == 0); // starts from the beginning of a byte

        const size_t ret = *reinterpret_cast<const size_t*>(origin + idx / 8);

        if (max_size >= sizeof(size_t) * 8) return ret;

        // max_size = 61 -> 3 to discard -> 0b1000 -> 0b111 -> 0b11...11000
        return ret & ~((1_zu << (sizeof(size_t) * 8 - max_size)) - 1);
    }

    bool FrameMask::operator[](const glm::ivec2 coords) const
    {
        assert(
            coords.x >= 0 && coords.x < size.x &&
            coords.y >= 0 && coords.y < size.y);

        const size_t idx = coords.x + coords.y * size.x;

        return origin[idx / 8] & (1 << (idx % 8));
    }



    bool SpriteDataRegistry::init()
    {
        return load_meta_json() && load_mask();
    }

    bool SpriteDataRegistry::load_meta_json()
    {
        /* JSON metadata format
         * {
         *   "atlases": [
         *     [
         *       {
         *         "sprite_id": <uint>,
         *         "anim_id": <uint>,
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
            printf(JSON_FMT_ERR_MSG("empty or nonexistent json file"));
            return false;
        }

        json j;

        try { j = json::parse(s.data()); }
        catch (const json::parse_error& e)
        {
            printf(JSON_FMT_ERR_MSG("%s"), e.what());
            return false;
        }

        if (!j.contains("atlases") || !j["atlases"].is_array())
        {
            printf(JSON_FMT_ERR_MSG("the json should contain \"atlases\": [...] ") "\n%s", j.dump().c_str());
            return false;
        }

        // "C++ is a strongly typed language, we shouldn't use auto"
        // - Abhishek
        for (decltype(*j.items().begin())& atlas : j["atlases"].items())
        {
            ++atlas_count_;

            if (!atlas.value().is_array())
            {
                printf(JSON_FMT_ERR_MSG("every member of \"atlases\": [...] should be an array") "\n%s", atlas.value().dump().c_str());
                return false;
            }

            for (decltype(*atlas.value().items().begin())& sprite : atlas.value().items())
            {
                const json& v = sprite.value();

                if (!v.contains("sprite_id") || !v["sprite_id"].is_number_unsigned() ||
                    !v.contains("anim_id") || !v["anim_id"].is_number_unsigned() ||
                    !v.contains("x") || !v["x"].is_number_unsigned() ||
                    !v.contains("y") || !v["y"].is_number_unsigned() ||
                    !v.contains("w") || !v["w"].is_number_unsigned() ||
                    !v.contains("h") || !v["h"].is_number_unsigned())
                {
                    printf(JSON_FMT_ERR_MSG("invalid sprite animation data format") "\n%s", v.dump().c_str());
                    return false;
                }

                anim::Info info {
                    v["sprite_id"].get<int>(),
                    v["anim_id"].get<uint32_t>()
                };

                SpriteAnimationData& data = anim_data_[anim::global_anim_id(info)];

                data.frame_count = anim::frame_count(info);
                data.offset      = { v["x"].get<uint32_t>(), v["y"].get<uint32_t>() };
                data.frame_size  = { v["w"].get<uint32_t>() / data.frame_count, v["h"].get<uint32_t>() };
                data.atlas_idx   = std::stoi(atlas.key());
            }
        }

        return true;
    }

    bool SpriteDataRegistry::load_mask()
    {
        masks_buf_ = FileIO::read<vector<uint8_t>>("texture_packer/mask");

        if (masks_buf_.empty())
        {
            printf("empty or nonexistent mask file");
            return false;
        }

        size_t offset = 0;
        for (uint32_t anim_id = 0; anim_id < anim::total_anim_count; ++anim_id)
        {
            const SpriteAnimationData& data = anim_data_[anim_id];

            for (uint32_t frame = 0; frame < data.frame_count; ++frame)
            {
                mask_offsets_.emplace_back(offset);
                offset += (data.frame_size.x * data.frame_size.y + 7) / 8;
            }
        }

        return true;
    }


    FrameMask SpriteDataRegistry::mask(const anim::Info info, const uint32_t frame) const
    {
        const SpriteAnimationData& data = anim_data(info);

        return {
            .origin = masks_buf_.data() + mask_offsets_[anim::global_frame_id(info, frame)],
            .size = data.frame_size
        };
    }

    const SpriteAnimationData& SpriteDataRegistry::anim_data(const anim::Info info) const
    {
        return anim_data_[anim::global_anim_id(info)];
    }
}
