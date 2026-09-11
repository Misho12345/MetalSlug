#include "sprite_manager.hpp"

#define ERR_MSG(MSG) "cannot load sprites, invalid json format - " MSG ";\n"

namespace mse
{
    bool SpriteManager::init()
    {
        for (size_t i = 0; i < anim::sprite_count; ++i)
        {
            sprite_anim_datas_[i].resize(anim::sprite_anim_count(i));
        }

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
                    !v.contains("x") || !v["x"].is_number_unsigned() ||
                    !v.contains("y") || !v["y"].is_number_unsigned() ||
                    !v.contains("w") || !v["w"].is_number_unsigned() ||
                    !v.contains("h") || !v["h"].is_number_unsigned())
                {
                    printf(ERR_MSG("invalid sprite animation data format") "\n%s", v.dump().c_str());
                    return false;
                }

                const uint32_t sprite_id = v["sprite_id"].get<uint32_t>();
                const uint32_t anim_id   = v["anim_id"].get<uint32_t>();
                const uint32_t x         = v["x"].get<uint32_t>();
                const uint32_t y         = v["y"].get<uint32_t>();
                const uint32_t w         = v["w"].get<uint32_t>();
                const uint32_t h         = v["h"].get<uint32_t>();

                SpriteAnimationData& anim_data = sprite_anim_datas_[sprite_id][anim_id];

                anim_data.offset      = { x, y };
                anim_data.image_size  = { w, h };
                anim_data.frame_count = 1;
                anim_data.atlas_idx   = std::stoi(atlas.key());
            }
        }

        return true;
    }

    void SpriteManager::update(const float dt)
    {
        size_t i = 0;
        for (list<Sprite>::node* n = sprites_.head(); n; n = n->next, ++i)
        {
            n->data.update(dt);
            if (n->data.dirty_) dirty_[i] = !(n->data.dirty_ = false);
        }
    }

    Sprite& SpriteManager::instantiate(int sprite_id, const SpriteDesc& desc)
    {
        sprites_.push_back({
            sprite_id,
            desc
        });

        dirty_.emplace_back(true);

        return sprites_.back();
    }

    void SpriteManager::destroy(const Sprite& sprite)
    {
        uint32_t idx = 0;
        for (list<Sprite>::node* n = sprites_.head(); n; n = n->next, ++idx)
        {
            if (&n->data == &sprite)
            {
                if (n != sprites_.tail())
                {
                    list<Sprite>::swap_elements(n, sprites_.tail());
                    dirty_[idx] = true;
                }

                sprites_.pop_back();
                return;
            }
        }

        assert(false);
    }

    void SpriteManager::set_frame_count(const anim::Info info, const uint32_t count)
    {
        sprite_anim_datas_[info.sprite_id][info.anim_id].frame_count = count;
    }

    const SpriteAnimationData& SpriteManager::anim_data(const anim::Info info) const
    {
        return sprite_anim_datas_[info.sprite_id][info.anim_id];
    }
}
