#pragma once
#include "mse/pch.hpp"

namespace mse
{
    /**
     * @brief One sprite as the sprite shader reads it
     * @details Layout has to match InstanceData in sprite.vert (std430)
     * @see RenderingSystem, ParticleSystem
     */
    struct alignas(8) InstanceData final
    {
        vec2 parallax_factor;

        ivec2 position;
        ivec2 scale;
        float rotation;

        uint32_t anim_idx;
        uint32_t frame_idx;

        uint32_t flip_x;

        uint32_t color;
    };
}
