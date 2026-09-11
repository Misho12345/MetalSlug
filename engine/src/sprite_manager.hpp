#pragma once
#include "mse/pch.hpp"
#include "mse/sprite.hpp"

namespace mse
{
    class SpriteManager final
    {
    public:
        SpriteManager() : sprite_anim_datas_(anim::sprite_count) {};

        bool init();
        void update(float dt);

        Sprite& instantiate(int sprite_id, const SpriteDesc& desc);
        void destroy(const Sprite& sprite);

        void set_frame_count(anim::Info info, uint32_t count);
        const SpriteAnimationData& anim_data(anim::Info info) const;

        const list<Sprite>& sprites() const { return sprites_; }
        vector<bool>& dirty() { return dirty_; }
        uint32_t atlas_count() const { return atlas_count_; }

    private:
        vector<vector<SpriteAnimationData>> sprite_anim_datas_{};

        list<Sprite> sprites_{};
        vector<bool> dirty_{};

        uint32_t atlas_count_{};
    };
}
