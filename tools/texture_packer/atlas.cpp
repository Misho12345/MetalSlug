#include "atlas.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

#include <stb_image.h>
#include <stb_image_write.h>


Box::Box(const char* image_path, const int sprite_id, const int anim_id)
    : sprite_id{ sprite_id },
      anim_id{ anim_id },
      image_path{ image_path }
{
    int t;
    data = reinterpret_cast<uint32_t*>(stbi_load(image_path, &w, &h, &t, 4));
}

Box::~Box()
{
    if (data) stbi_image_free(data);
}


void Atlas::add(Box* box)
{
    boxes_.emplace_back(box);
}

void Atlas::save(const char* path) const
{
    uint32_t* data = new uint32_t[SIZE * SIZE];

    for (size_t i = 0; i < boxes_.size(); ++i)
    {
        const Box& box = *boxes_[i];
        const bool rotated = box.x < 0;
        const int bx = rotated ? -box.x : box.x;

        const uint32_t* p = box.data;

        if (rotated)
        {
            for (int x = 0; x < box.h; ++x)
            {
                for (int y = 0; y < box.w; ++y, ++p)
                {
                    data[(bx + x + (box.y + y) * SIZE)] = *p;
                }
            }
        }
        else
        {
            for (int y = 0; y < box.h; ++y, p += box.w)
            {
                memcpy(data + (bx + (box.y + y) * SIZE), p, box.w * sizeof(uint32_t));
            }
        }
    }

    stbi_write_png(path, SIZE, SIZE, sizeof(uint32_t), data, SIZE * sizeof(uint32_t));
    delete[] data;
}
