#include "mse/pch.hpp"
#include "mse/ecs/components.hpp"

#include "priv_ctx.hpp"
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

    glm::ivec2 SpriteRenderer::size() const
    {
        return App::priv_ctx().sprite_data_registry.anim_data(info_).frame_size;
    }

    aabb SpriteRenderer::bounds(const Transform& tr) const
    {
        const glm::ivec2 s = size() * tr.scale;
        const glm::ivec2 hs = s / 2;
        return aabb{ -hs, s - hs } + tr.position;
    }

    aabb SpriteRenderer::screen_bounds(const Transform& tr, const glm::ivec2 cam_pos) const
    {
        return bounds(tr) - glm::ivec2(glm::vec2(cam_pos) * parallax_factor);
    }


    SpriteCollider::SpriteCollider(const entity_id id)
    {
        const SpriteRenderer* sr = App::ctx().scene.try_get<SpriteRenderer>(id);
        if (!sr) return;
        size = sr->size();
    }

    SpriteCollider::SpriteCollider(const glm::ivec2 _offset, const glm::ivec2 _size)
    {
        offset = _offset;
        size = _size;
    }

    aabb SpriteCollider::bounds(const Transform& tr) const
    {
        const glm::ivec2 s = size * tr.scale;
        const glm::ivec2 hs = s / 2;
        return aabb{ -hs, s - hs } + (offset + tr.position);
    }
}
