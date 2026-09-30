#pragma once
#include <cstdint>
#include "common/common.hpp"

struct Box final
{
    Box(const char* _image_path, uint32_t _sprite_id, uint32_t _anim_id, uint32_t _frame_count);
    ~Box();

    Box(const Box&)            = delete;
    Box& operator=(const Box&) = delete;

    Box(Box&& other) noexcept;
    Box& operator=(Box&& other) noexcept;

    const char* image_path;
    uint32_t* data{};

    uint32_t x{}, y{};
    uint32_t w{}, h{};

    uint32_t sprite_id, anim_id;

    // frame count is still passed and stored, even though the program doesn't save it in meta.json
    // because it's still needed for the mask file generation
    uint32_t frame_count;

    bool rotated{ false };
};

class Atlas final
{
public:
    static constexpr uint32_t SIZE = 2048;

    Atlas() = default;
    ~Atlas() = default;

    void add(Box* box);
    void save(const char* atlas_path) const;

private:
    mse::vector<Box*>  boxes_{ nullptr };
};

void save_masks(mse::span<const Box> boxes, const char* mask_path);
