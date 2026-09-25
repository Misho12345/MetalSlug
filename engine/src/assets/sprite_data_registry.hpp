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

    /**
     * @brief Alpha mask for a singular frame
     * @details Has a pointer to the mask and the size of the frame
     */
    struct FrameMask final
    {
        // The mask has packed data for weather a pixel is transparent or not
        // 1 byte contains the data for 8 consecutive pixels
        // So if the frame size is 26x15 => origin will be 49 bytes
        // The data is LSB-first
        //   so for pixels 0..31 this is what the buffer would look like:
        //   [0] 7 6 .. 0   [1] 15 14 .. 8  [2]  23 22 .. 16   [3] 31 30 .. 24
        //
        // NOTE: rows are not padded, so a byte can contain the data for the
        //       last pixels in a row and the first pixels in the next one
        const uint8_t* origin;

        // struct will be 16 bytes either way because of padding, might as well store the height for assert checks
        glm::ivec2 size;

        // returns a strip from the mask to compare with another one
        // the strip is of size sizeof(size_t), therefore containing data for 64 pixels for x64
        // assumes coords is a position that lies on the start of a byte, not in the middle
        //
        size_t range(glm::ivec2 coords, uint32_t max_size) const;

        bool operator[](glm::ivec2 coords) const;
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
