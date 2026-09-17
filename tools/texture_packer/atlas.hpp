#pragma once
#include <cstdint>
#include "common/common.hpp"

struct Box final
{
    Box(const char* image_path, int sprite_id, int anim_id, int frame_count);
    ~Box();

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
    void save(const char* path) const;

private:
    mse::vector<Box*>  boxes_{ nullptr };
};
