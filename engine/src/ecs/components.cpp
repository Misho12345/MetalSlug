#include "mse/pch.hpp"
#include "mse/ecs/components.hpp"

#include "priv_ctx.hpp"
#include "mse/app.hpp"

namespace mse
{
    void Sprite::play(
        const uint32_t anim_id,
        const bool     loop,
        const bool     restart_if_same)
    {
        if (anim_id != info_.anim_id || restart_if_same)
        {
            time_         = 0.0f;
            frame_        = 0;
            info_.anim_id = anim_id;
        }

        loop_ = loop;
        paused = false;
        anim_done_ = false;
    }

    void Sprite::stop()
    {
        paused = true;
        time_  = 0.0f;
        frame_ = 0;
    }



    uvec2 Sprite::size() const
    {
        return App::priv_ctx().sprite_data_registry.anim_data(info_).frame_size;
    }

    aabb Sprite::bounds(const Transform& tr) const
    {
        const ivec2 s = size() * tr.scale;
        const ivec2 hs = s / 2;
        return aabb{ -hs, s - hs } + tr.position;
    }

    aabb Sprite::screen_bounds(const Transform& tr, const ivec2 cam_pos) const
    {
        return bounds(tr) - ivec2(vec2(cam_pos) * parallax_factor);
    }

    Sprite::Sprite(const Scene& scene, const entity_id id, const anim::Info info) : info_(info)
    {
        assert(scene.has<Transform>(id) && "Sprite requires a Transform component");
    }



    Collider::Collider(const Scene& scene, const entity_id id)
    {
        assert(scene.has<Transform>(id) && "Collider requires a Transform component");
        size = scene.get<Sprite>(id).size();
    }

    Collider::Collider(const Scene& scene, const entity_id id, const ivec2 _offset, const ivec2 _size)
    {
        assert(scene.has<Transform>(id) && "Collider requires a Transform component");

        offset = _offset;
        size = _size;
    }

    aabb Collider::bounds(const Transform& tr) const
    {
        const ivec2 s = size * tr.scale;
        const ivec2 hs = s / 2;
        return aabb{ -hs, s - hs } + (offset + tr.position);
    }
}
