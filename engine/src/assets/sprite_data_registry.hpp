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
        glm::ivec2 offset;
        glm::ivec2 frame_size;
        uint32_t   frame_count;
        uint32_t   atlas_idx;
    };

    struct FrameMask final
    {
        const uint8_t* origin;

        // struct will be 16 bytes either way because of padding, might as well store the height for assert checks
        glm::ivec2 size;

        // gets a strip from the mask to compare with another one
        size_t range(const glm::ivec2 coords, const uint32_t max_size) const
        {
            assert(
                coords.x >= 0 && coords.x < size.x &&
                coords.y >= 0 && coords.y < size.y);

            assert(size.x - coords.x - 1u <= max_size); // does not leak to the next row

            const size_t idx = coords.x + coords.y * size.x;
            assert(idx % 8); // starts from the beginning of a byte

            const size_t ret = *reinterpret_cast<const size_t*>(origin + idx / 8);

            if (max_size >= sizeof(size_t) * 8) return ret;

            // max_size = 61 -> 3 to discard -> 0b1000 -> 0b111 -> 0b11...11000
            return ret & ~((1 << (sizeof(size_t) * 8 - max_size)) - 1);
        }
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

        [[nodiscard]] FrameMask mask(anim::Info info, uint32_t frame) const;
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
