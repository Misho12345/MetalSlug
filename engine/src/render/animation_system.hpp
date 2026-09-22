#pragma once
#include "mse/pch.hpp"
#include "mse/ecs/scene.hpp"
#include "assets/sprite_data_registry.hpp"

namespace mse
{
    struct PrivCtx;

    /**
     * @brief Animation system
     * @details This system is responsible for loading the animation data from texture_packer output and
     * updating the animation data
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class AnimationSystem final
    {
    public:
        AnimationSystem(const AnimationSystem&)            = delete;
        AnimationSystem(AnimationSystem&&)                 = delete;
        AnimationSystem& operator=(const AnimationSystem&) = delete;
        AnimationSystem& operator=(AnimationSystem&&)      = delete;

        void update(Scene& scene, float dt) const;

    private:
        AnimationSystem() = default;

        friend ::mse::PrivCtx;
    };
}
