#pragma once
#include "pch.hpp"
#include "aabb.hpp"

namespace mse
{
    struct Transform final
    {
        glm::vec2 position{ 0.0f, 0.0f };
        glm::vec2 scale{ 100.0f, 100.0f };
        float     rotation{ 0.0f };

        constexpr glm::vec2 top_left() const { return position - scale * 0.5f; }
        constexpr glm::vec2 bottom_right() const { return position + scale * 0.5f; }

        aabb bounds() const { return aabb{ top_left(), bottom_right() }; }
    };

    struct Rigidbody final
    {
        void apply_force(const glm::vec2 force) { acceleration += force / mass; }

        glm::vec2 velocity{ 0.0f, 0.0f };
        glm::vec2 acceleration{ 0.0f, 0.0f };

        float mass{ 1.0f };
        float drag{ 0.02f };

        bool gravity{ true };
        bool is_grounded{ false };
    };

    struct BoxCollider final
    {
        glm::vec2 offset{ 0.0f, 0.0f };
        glm::vec2 size{ 32.0f, 32.0f };
        bool      is_trigger{ false };

        aabb bounds(const Transform& tr) const
        {
            const glm::vec2 top_left = tr.top_left() + offset;
            return aabb{ top_left, top_left + size };
        }
    };

    struct MSE_API SpriteRenderer final
    {
        template <anim::sprite_enum E>
        SpriteRenderer(E animation) : info_{ anim::sprite_id<E>, static_cast<uint32_t>(animation) } {}

        glm::vec2 parallax_factor{ 0.0f };

        float frame_dur{ 0.15f };
        int32_t layer{ 0 };

        bool paused{ false };
        bool hidden{ false };

        anim::Info info() const { return info_; }
        uint32_t   frame() const { return frame_; }

        template <anim::sprite_enum E>
        void play(const E animation, const bool restart_if_same = false)
        {
            assert(anim::sprite_id<E> == info_.sprite_id && "SpriteRenderer::play: sprite_id mismatch");
            play(static_cast<uint32_t>(animation), restart_if_same);
        }

        void stop();
        void update(float dt, uint32_t frame_count);

    private:
        void play(uint32_t anim_id, bool restart_if_same);

        anim::Info info_;

        float    time_{ 0.0f };
        uint32_t frame_{ 0 };
    };
}
