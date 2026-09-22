#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct PrivCtx;

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

    struct FrameMask final
    {
        const uint8_t* origin;
        glm::ivec2 size;
    };

    /**
     * @brief Manages sprite animation data and alpha masks (for pixel perfect checks)
     * @detail Loads meta.json and mask from tools/texture_packer
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class SpriteDataRegistry final
    {
    public:
        SpriteDataRegistry(const SpriteDataRegistry&)            = delete;
        SpriteDataRegistry(SpriteDataRegistry&&)                 = delete;
        SpriteDataRegistry& operator=(const SpriteDataRegistry&) = delete;
        SpriteDataRegistry& operator=(SpriteDataRegistry&&)      = delete;

        // load the animation data and masks buffer from texture_packer output
        bool init();

        [[nodiscard]] FrameMask mask(anim::Info info, uint32_t frame);
        [[nodiscard]] const SpriteAnimationData& anim_data(anim::Info info) const;

        const vector<SpriteAnimationData>& anim_data() const { return anim_data_; }

        [[nodiscard]]
        uint32_t atlas_count() const { return atlas_count_; }

    private:
        SpriteDataRegistry();

        bool load_meta_json();
        bool load_mask();

        vector<SpriteAnimationData> anim_data_{}; // key: global anim id
        vector<size_t> mask_offsets_{};           // key: global frame id
        vector<uint8_t> masks_buf_{ nullptr };

        uint32_t atlas_count_{};

        friend ::mse::PrivCtx;
    };
}
