#include "mse/sprite.hpp"
#include "priv_ctx.hpp"
#include "mse/app.hpp"

namespace mse
{
    void Sprite::destroy() const
    {
        App::priv_ctx().sprite_manager.destroy(*this);
    }

    void Sprite::update(const float dt)
    {
        const uint32_t frame_count = App::priv_ctx().sprite_manager.anim_data(info_).frame_count;

        if (paused_ || frame_dur == 0.0f || frame_count <= 1) return;

        time_ += dt;
        const uint32_t frame = frame_;

        while (time_ > frame_dur)
        {
            time_ -= frame_dur;
            ++frame_;
        }

        frame_ %= frame_count;
        if (frame_ != frame) dirty_ = true;
    }

    Sprite::Sprite(
        const int         sprite_id,
        const SpriteDesc& desc)
        : pos_{ desc.pos },
          size_{ desc.size },
          rot_{ desc.rot },
          info_{ sprite_id, 0 } {}

    Sprite& Sprite::create(
        const int         sprite_id,
        const SpriteDesc& desc)
    {
        return App::priv_ctx().sprite_manager.instantiate(
            sprite_id,
            desc
        );
    }


    void Sprite::set_frame_count(const anim::Info info, const uint32_t count)
    {
        App::priv_ctx().sprite_manager.set_frame_count(info, count);
    }

    void Sprite::play(const int anim_id)
    {
        if (anim_id == -1)
        {
            paused_ = false;
            return;
        }

        time_  = 0.0f;
        frame_ = 0;
        info_.anim_id = anim_id;

        dirty_ = true;
    }

    void Sprite::pause() { paused_ = true; }
}
