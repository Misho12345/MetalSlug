#pragma once
#include <cstdint>
#include "common/common.hpp"

struct Box final
{
    Box(const char* image_path, int sprite_id, int anim_id);
    ~Box();

    int x{}, y{}; // x sign bit is flag for rotated
    int w{}, h{};

    int sprite_id{}, anim_id{};

    const char* image_path{};
    uint32_t* data{};
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
