#pragma once
#include "mse/pch.hpp"
#include "mse/scene.hpp"

namespace mse
{
    /**
     * @brief Sprite animation data
     * @details This is the format of the animation data for the buffer sprite rendering
     */
    struct SpriteAnimationData final
    {
        glm::vec2 offset;
        glm::vec2 image_size;
        uint32_t  frame_count;
        uint32_t  atlas_idx;
    };

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

        bool init(); // load the animation data from texture_packer output
        void update(Scene& scene, float dt) const;

        const SpriteAnimationData& anim_data(anim::Info info) const;
        uint32_t atlas_count() const { return atlas_count_; }

    private:
        AnimationSystem() : sprite_anim_datas_(anim::sprite_count) {}

        vector<vector<SpriteAnimationData>> sprite_anim_datas_{};
        uint32_t atlas_count_{};

        friend ::mse::PrivCtx;
    };
}
