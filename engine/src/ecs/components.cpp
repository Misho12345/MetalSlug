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

    ivec2 SpriteRenderer::size() const
    {
        return App::priv_ctx().sprite_data_registry.anim_data(info_).frame_size;
    }

    aabb SpriteRenderer::bounds(const Transform& tr) const
    {
        const ivec2 s = size() * ivec2(tr.scale);
        const ivec2 hs = s / 2;
        return aabb{ -hs, s - hs } + tr.position;
    }

    aabb SpriteRenderer::screen_bounds(const Transform& tr, const ivec2 cam_pos) const
    {
        return bounds(tr) - ivec2(vec2(cam_pos) * parallax_factor);
    }

    SpriteRenderer::SpriteRenderer(const Scene& scene, const entity_id id, const anim::Info info) : info_(info)
    {
        assert(scene.has<Transform>(id) && "SpriteRenderer requires a Transform component");
    }


    SpriteCollider::SpriteCollider(const Scene& scene, const entity_id id)
    {
        assert(scene.has<Transform>(id) && "SpriteCollider requires a Transform component");
        assert(scene.has<SpriteRenderer>(id) && "SpriteCollider requires a SpriteRenderer component");

        size = scene.get<SpriteRenderer>(id).size();
    }

    SpriteCollider::SpriteCollider(const Scene& scene, const entity_id id, const ivec2 _offset, const ivec2 _size)
    {
        assert(scene.has<Transform>(id) && "SpriteCollider requires a Transform component");
        assert(scene.has<SpriteRenderer>(id) && "SpriteCollider requires a SpriteRenderer component");

        offset = _offset;
        size = _size;
    }

    aabb SpriteCollider::bounds(const Transform& tr) const
    {
        const ivec2 s = size * tr.scale;
        const ivec2 hs = s / 2;
        return aabb{ -hs, s - hs } + (offset + tr.position);
    }
}
