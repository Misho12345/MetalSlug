#pragma once
#include "mse/api.hpp"
#include "mse/pch.hpp"

#include "mse/render/instance_data.hpp"

namespace mse
{
    struct GameCtx;
    class RenderingSystem;

    struct ParticleDesc final
    {
        vec2  position{}; // centre of the frame, like Transform
        vec2  velocity{}; // px/s
        float gravity{};  // px/s^2

        uvec2 scale{ 1 };
        vec2  parallax_factor{ 1.0f, 1.0f };

        // ticks. 0 lives for one play of the animation, anything else loops it until the life runs out
        uint16_t life{};
        uint8_t  frame_dur{};

        int32_t layer{};
        bool    flip_x{};
        color_t color{};
    };

    /**
     * @brief Sprites that only move and animate, without being entities
     * @details Each particle is kept as the InstanceData the sprite shader reads, so drawing them is a copy
     * into the instance buffer. What the shader doesn't need (velocity, life, animation timing) sits in a
     * second array at the same index. Dead particles are swapped with the last one and popped
     * @note Owned by GameCtx, not constructable by anything else and not movable or copyable
     */
    class MSE_API ParticleSystem final
    {
    public:
        ParticleSystem(const ParticleSystem&)            = delete;
        ParticleSystem(ParticleSystem&&)                 = delete;
        ParticleSystem& operator=(const ParticleSystem&) = delete;
        ParticleSystem& operator=(ParticleSystem&&)      = delete;

        template <anim::sprite_enum E>
        void emit(const E animation, const ParticleDesc& desc) { emit(anim::info(animation), desc); }

        void emit(anim::Info info, const ParticleDesc& desc);

        void update();
        void clear();

        /// @brief Frame size of an animation, for placing a particle by something other than its centre
        template <anim::sprite_enum E>
        [[nodiscard]]
        static uvec2 frame_size(const E animation) { return frame_size(anim::info(animation)); }

        [[nodiscard]]
        static uvec2 frame_size(anim::Info info);

    private:
        ParticleSystem() = default;

        struct Motion final
        {
            vec2  position; // sub-pixel, InstanceData::position is this floored
            vec2  velocity;
            float gravity;

            uint16_t life;
            uint8_t  frame_dur;
            uint8_t  time;
            uint32_t frame_count;
        };

        // one per sprite layer, so they draw in between the sprites
        struct Layer final
        {
            int32_t layer;

            vector<InstanceData> instances;
            vector<Motion>       motion;
        };

        vector<Layer> layers_{ nullptr };

        friend ::mse::GameCtx;
        friend ::mse::RenderingSystem;
    };
}
