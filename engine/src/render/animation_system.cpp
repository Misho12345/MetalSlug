#include "mse/pch.hpp"
#include "animation_system.hpp"

#include "priv_ctx.hpp"
#include "mse/ecs/components.hpp"
#include "mse/app.hpp"


namespace mse
{
    void AnimationSystem::update(Scene& scene) const
    {
        for (Sprite& sprite : scene.pool<Sprite>().components())
        {
            const uint32_t frame_count = anim::frame_count(sprite.info_);

            if (sprite.hidden || sprite.paused || sprite.anim_done_ ||
                (sprite.frame_dur_ == 0 && sprite.frame_durs_.empty()) ||
                (sprite.loop_ && frame_count <= 1))
                continue;

            const uint8_t frame_dur = sprite.frame_durs_.empty()
                                          ? sprite.frame_dur_
                                          : sprite.frame_durs_[sprite.frame_];

            if (sprite.time_ + 1 < frame_dur)
            {
                ++sprite.time_;
                continue;
            }

            sprite.time_ = 0;
            
            if (sprite.reversed_)
            {
                if (sprite.frame_ > 0) --sprite.frame_;
                else if (sprite.loop_) sprite.frame_ = frame_count - 1;
                else sprite.anim_done_ = true;
            }
            else
            {
                if (sprite.frame_ + 1 < frame_count) ++sprite.frame_;
                else if (sprite.loop_) sprite.frame_ = 0;
                else sprite.anim_done_ = true;
            }
        }
    }
}