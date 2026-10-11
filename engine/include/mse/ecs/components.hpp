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
        float rotation{ 0.0f };
    };

    using should_loop = bool_value<struct loop_tag>;
    using should_restart_if_same = bool_value<struct restart_tag>;

    /**
     * @brief Sprite information for rendering
     * @note If present and not hidden, CollisionSystem will use the sprite's mask for pixel perfect collision check
     * @see tools/texture_packer, sprite_data_registry.hpp, collision_system.cpp
     */
    struct MSE_API Sprite final
    {
        template <anim::sprite_enum E>
        Sprite(const Scene& scene, const entity_id id, E animation)
            : Sprite(scene, id, anim::info(animation)) {}

        vec2 parallax_factor{ 1.0f, 1.0f };
        int32_t layer{ 0 };

        bool paused{ false };
        bool hidden{ false };
        bool flip_x{ false };

        [[nodiscard]] anim::Info info() const { return info_; }
        [[nodiscard]] uint32_t   frame() const { return frame_; }

        /**
         * @brief Plays animation
         * @tparam E Animation enum type (deduced)
         * @param animation Animation, must belong to the same enum the component was initialized with
         * @param frame_dur Duration of each frame in ticks
         * @param loop Whether the animation should loop or stop after the last frame is played
         * @param restart_if_same Whether to set the frame to 0 if the animation is the same
         */
        template <anim::sprite_enum E>
        void play(const E                      animation,
                  const uint8_t                frame_dur       = 0,
                  const should_loop            loop            = should_loop::no,
                  const should_restart_if_same restart_if_same = should_restart_if_same::no)
        {
            assert(anim::sprite_id<E> == info_.sprite_id && "Sprite::play: sprite_id mismatch");

            frame_durs_.clear();
            frame_dur_ = frame_dur;

            play((uint32_t)animation, loop, restart_if_same);
        }

        /**
         * @brief Plays animation
         * @tparam E Animation enum type (deduced)
         * @param animation Animation, must belong to the same enum the component was initialized with
         * @param frame_durs An array with the duration of each frame in ticks, it must be with the same size as the number of frames
         * @param loop Whether the animation should loop or stop after the last frame is played
         * @param restart_if_same Whether to set the frame to 0 if the animation is the same
         */
        template <anim::sprite_enum E, size_t N>
        void play(const E                      animation,
                  const uint8_t (&               frame_durs)[N],
                  const should_loop            loop            = should_loop::no,
                  const should_restart_if_same restart_if_same = should_restart_if_same::no)
        {
            assert(anim::sprite_id<E> == info_.sprite_id && "Sprite::play: sprite_id mismatch");
            assert(anim::frame_count(anim::info(animation)) == N);

            frame_durs_ = frame_durs;
            play((uint32_t)animation, loop, restart_if_same);
        }

        void stop();

        [[nodiscard]] uvec2 size() const;
        [[nodiscard]] aabb bounds(const Transform& tr) const;
        [[nodiscard]] aabb screen_bounds(const Transform& tr, ivec2 cam_pos) const;

        [[nodiscard]]
        bool anim_done() const { return anim_done_; }

    private:
        Sprite(const Scene& scene, entity_id id, anim::Info info);

        void play(uint32_t anim_id, bool loop, bool restart_if_same);

        anim::Info info_;
        uint32_t frame_{ 0 };

        vector<uint8_t> frame_durs_{ nullptr };

        uint8_t frame_dur_{ 10 };
        uint8_t time_{ 0 };

        bool loop_{ false };
        bool anim_done_{ false };

        friend class AnimationSystem;
    };


    struct MSE_API Collider final
    {
        explicit Collider(const Scene& scene, entity_id id);
        Collider(const Scene& scene, entity_id id, ivec2 _offset, ivec2 _size);

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
        bool grounded{ false };
        bool enabled{ true };
    };
}
