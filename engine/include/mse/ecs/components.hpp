#pragma once
#include "mse/pch.hpp"

#include "entity_id.hpp"
#include "mse/collision/aabb.hpp"

namespace mse
{
    class Scene;

    struct Transform final
    {
        ivec2 position{ 0, 0 };
        uvec2 scale{ 1 };
        float      rotation{ 0.0f };
    };


    struct MSE_API SpriteRenderer final
    {
        template <anim::sprite_enum E>
        SpriteRenderer(const Scene& scene, const entity_id id, E animation)
            : SpriteRenderer(scene, id, anim::info(animation)) {}

        vec2 parallax_factor{ 1.0f, 1.0f };

        float frame_dur{ 0.1f };
        int32_t layer{ 0 };

        bool paused{ false };
        bool hidden{ false };
        bool flip_x{ false };

        [[nodiscard]] anim::Info info() const { return info_; }
        [[nodiscard]] uint32_t   frame() const { return frame_; }

        template <anim::sprite_enum E>
        void play(const E animation, const bool restart_if_same = false)
        {
            assert(anim::sprite_id<E> == info_.sprite_id && "SpriteRenderer::play: sprite_id mismatch");
            play((uint32_t)animation, restart_if_same);
        }

        void stop();
        void update(float dt, uint32_t frame_count);

        [[nodiscard]] uvec2 size() const;
        [[nodiscard]] aabb bounds(const Transform& tr) const;
        [[nodiscard]] aabb screen_bounds(const Transform& tr, ivec2 cam_pos) const;

    private:
        SpriteRenderer(const Scene& scene, entity_id id, anim::Info info);

        void play(uint32_t anim_id, bool restart_if_same);

        anim::Info info_;

        float    time_{ 0.0f };
        uint32_t frame_{ 0 };
    };


    struct MSE_API SpriteCollider final
    {
        explicit SpriteCollider(const Scene& scene, entity_id id);
        SpriteCollider(const Scene& scene, entity_id id, ivec2 _offset, ivec2 _size);

        [[nodiscard]]
        aabb bounds(const Transform& tr) const;

        vec2 velocity{ 0.0f, 0.0f };
        vec2 move{};

        float gravity{ 900.0f };
        float max_fall_speed{ 300.0f };

        ivec2 offset{};
        uvec2 size{ 32 };

        uint32_t layer{};
        uint32_t target_layer{};

        void (*callback)(entity_id, uint32_t){ nullptr };

        bool collide_with_tile_map = true;

        // set by collision system
        bool grounded = false;
    };
}
