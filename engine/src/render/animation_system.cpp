#include "mse/pch.hpp"
#include "animation_system.hpp"

#include "priv_ctx.hpp"
#include "mse/ecs/components.hpp"
#include "mse/app.hpp"


namespace mse
{
    void AnimationSystem::update(Scene& scene) const
    {
        const SpriteDataRegistry& reg = App::priv_ctx().sprite_data_registry;

        for (Sprite& sprite : scene.pool<Sprite>().components())
        {
            const uint32_t frame_count = reg.anim_data(sprite.info_).frame_count;

            if (sprite.hidden || sprite.anim_done_ || sprite.paused ||
                (sprite.frame_dur_ == 0 && sprite.frame_durs_.empty()) ||
                frame_count <= 1)
                continue;

            const uint8_t frame_dur = sprite.frame_durs_.empty()
                                          ? sprite.frame_dur_
                                          : sprite.frame_durs_[sprite.frame_];

            if (++sprite.time_ <= frame_dur) continue;

            sprite.time_ = 0;

            if (sprite.frame_ >= frame_count)
            {
                if (sprite.loop_) sprite.anim_done_ = true;
                else sprite.frame_ = 0;
            }
            else ++sprite.frame_;
        }
    }
}