#pragma once
#include "entity_id.hpp"
#include "mse/pch.hpp"
#include "mse/collision/aabb.hpp"

namespace mse
{
    struct Transform final
    {
        glm::ivec2 position{ 0, 0 };
        glm::ivec2 scale{ 1 };
        float      rotation{ 0.0f };
    };

    struct MSE_API SpriteRenderer final
    {
        template <anim::sprite_enum E>
        SpriteRenderer(E animation) : info_(anim::info(animation)) {}

        glm::vec2 parallax_factor{ 0.0f };

        float frame_dur{ 0.15f };
        int32_t layer{ 0 };

        bool paused{ false };
        bool hidden{ false };

        [[nodiscard]] anim::Info info() const { return info_; }
        [[nodiscard]] uint32_t   frame() const { return frame_; }

        template <anim::sprite_enum E>
        void play(const E animation, const bool restart_if_same = false)
        {
            assert(anim::sprite_id<E> == info_.sprite_id && "SpriteRenderer::play: sprite_id mismatch");
            play(static_cast<uint32_t>(animation), restart_if_same);
        }

        void stop();
        void update(float dt, uint32_t frame_count);

        [[nodiscard]] glm::ivec2 size() const;
        [[nodiscard]] aabb bounds(const Transform& tr) const;
        [[nodiscard]] aabb screen_bounds(const Transform& tr, glm::ivec2 cam_pos) const;

    private:
        void play(uint32_t anim_id, bool restart_if_same);

        anim::Info info_;

        float    time_{ 0.0f };
        uint32_t frame_{ 0 };
    };

    struct MSE_API SpriteCollider final
    {
        explicit SpriteCollider(entity_id id);
        SpriteCollider(glm::ivec2 _offset, glm::ivec2 _size);

        void apply_force(const glm::vec2 force) { acceleration += force / mass; }

        [[nodiscard]]
        aabb bounds(const Transform& tr) const;

        glm::vec2 velocity{ 0.0f, 0.0f };
        glm::vec2 acceleration{ 0.0f, 0.0f };
        glm::vec2 pos_remainder{};
        float mass{ 1.0f };
        float drag{ 0.02f };

        glm::ivec2 offset{};
        glm::ivec2 size{ 32 };
    };
}
