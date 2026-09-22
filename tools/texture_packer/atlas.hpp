#pragma once
#include <cstdint>
#include "common/common.hpp"

struct Box final
{
    Box(const char* _image_path, int _sprite_id, int _anim_id, int _frame_count);
    ~Box();

    Box(const Box&)            = delete;
    Box& operator=(const Box&) = delete;

    Box(Box&& other) noexcept;
    Box& operator=(Box&& other) noexcept;

    const char* image_path;
    uint32_t* data{};

    int x{}, y{};
    int w{}, h{};

    int sprite_id, anim_id;
    int frame_count;

    bool rotated{ false };
};

class Atlas final
{
public:
    static constexpr int SIZE = 2048;

    Atlas() = default;
    ~Atlas() = default;

    void add(Box* box);
    void save(const char* atlas_path) const;

    static void save_masks(mse::span<const Atlas> atlases, const char* mask_path);

private:
    mse::vector<Box*>  boxes_{ nullptr };
};
