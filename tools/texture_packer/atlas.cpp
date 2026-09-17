#include "atlas.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

#include <stb_image.h>
#include <stb_image_write.h>


Box::Box(const char* image_path, const int sprite_id, const int anim_id, const int frame_count)
    : image_path{ image_path },
      sprite_id{ sprite_id },
      anim_id{ anim_id },
      frame_count{ frame_count }
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
    uint32_t* data = new uint32_t[SIZE * SIZE]();

    for (const Box* box : boxes_)
    {
        const uint32_t* p = box->data;

        if (box->rotated)
        {
            // copy pixel by pixel because it's rotated
            for (int x = 0; x < box->h; ++x)
            {
                for (int y = 0; y < box->w; ++y, ++p)
                {
                    data[(box->x + x + (box->y + y) * SIZE)] = *p;
                }
            }
        }
        else
        {
            // copy row by row
            for (int y = 0; y < box->h; ++y, p += box->w)
            {
                memcpy(data + (box->x + (box->y + y) * SIZE), p, box->w * sizeof(uint32_t));
            }
        }
    }

    stbi_write_png(path, SIZE, SIZE, sizeof(uint32_t), data, SIZE * sizeof(uint32_t));
    delete[] data;
}
