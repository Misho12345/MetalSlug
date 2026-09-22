#include "mse/pch.hpp"
#include "mse/ecs/components.hpp"

#include "mse/app.hpp"

namespace mse
{
    void SpriteRenderer::play(const uint32_t anim_id, const bool restart_if_same)
    {
        if (anim_id != info_.anim_id || restart_if_same)
        {
            time_         = 0.0f;
            frame_        = 0;
            info_.anim_id = anim_id;
        }

        paused = false;
    }

    void SpriteRenderer::stop()
    {
        paused = true;
        time_  = 0.0f;
        frame_ = 0;
    }

    void SpriteRenderer::update(const float dt, const uint32_t frame_count)
    {
        if (paused || frame_dur == 0.0f || frame_count <= 1) return;

        time_ += dt;

        while (time_ > frame_dur)
        {
            time_ -= frame_dur;
            ++frame_;
        }

        frame_ %= frame_count;
    }



    SpriteCollider::SpriteCollider(const entity_id id)
    {
        const Transform* tr = App::ctx().scene.try_get<Transform>(id);
        if (!tr) return;
        size = tr->scale;
    }

    SpriteCollider::SpriteCollider(const glm::ivec2 _offset, const glm::ivec2 _size)
    {
        offset = _offset;
        size = _size;
    }
}
