#include "atlas.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

#include <stb_image.h>
#include <stb_image_write.h>


Box::Box(const char* _image_path, const int _sprite_id, const int _anim_id, const int _frame_count)
    : image_path{ _image_path },
      sprite_id{ _sprite_id },
      anim_id{ _anim_id },
      frame_count{ _frame_count }
{
    int t;
    data = reinterpret_cast<uint32_t*>(stbi_load(image_path, &w, &h, &t, 4));
}

Box::~Box() { if (data) stbi_image_free(data); }

Box::Box(Box&& other) noexcept :
    image_path{ std::exchange(other.image_path, nullptr) },
    data{ std::exchange(other.data, nullptr) },
    x{ other.x },
    y{ other.y },
    w{ other.w },
    h{ other.h },
    sprite_id{ other.sprite_id },
    anim_id{ other.anim_id },
    frame_count{ other.frame_count },
    rotated{ other.rotated } {}

Box& Box::operator=(Box&& other) noexcept
{
    if (this == &other) return *this;

    if (data) stbi_image_free(data);

    image_path = std::exchange(other.image_path, nullptr);
    data       = std::exchange(other.data, nullptr);

    x = other.x;
    y = other.y;
    w = other.w;
    h = other.h;

    sprite_id   = other.sprite_id;
    anim_id     = other.anim_id;
    frame_count = other.frame_count;
    rotated     = other.rotated;

    return *this;
}


void Atlas::add(Box* box) { boxes_.emplace_back(box); }

void Atlas::save(const char* atlas_path) const
{
    uint32_t* data = new uint32_t[SIZE * SIZE]();

    // save to texture atlas
    for (const Box* box : boxes_)
    {
        const uint32_t* p = box->data;

        if (box->rotated)
        {
            // copy pixel by pixel because it's rotated
            for (int x = 0; x < box->h; ++x)
            {
                for (int y = 0; y < box->w; ++y, ++p) { data[(box->x + x + (box->y + y) * SIZE)] = *p; }
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

    stbi_write_png(atlas_path, SIZE, SIZE, sizeof(uint32_t), data, SIZE * sizeof(uint32_t));
    delete[] data;
}

void save_masks(const mse::span<const Box> boxes, const char* mask_path)
{
    size_t total_size = 0;

    for (const Box& box : boxes)
    {
        // round up to the count of bytes
        total_size += (box.w * box.h + 7) / 8;
    }

    uint8_t* data = new uint8_t[total_size]();
    uint8_t* dp = data;
    int bit = 0;

    for (const Box& box : boxes)
    {
        // for each box go through each frame and put the data of the frames sequentially
        // that way when loaded each frame will have a pointer for the it's mask, and not have to find
        // the parts of the mask for the specific frame in the entire animation data block

        const uint32_t* p       = box.data;
        const size_t    frame_w = box.w / box.frame_count;

        for (int f = 0; f < box.frame_count; ++f)
        {
            const size_t off_x = frame_w * f;
            for (int y = 0; y < box.h; ++y)
            {
                for (int x = 0; x < frame_w; ++x)
                {
                   const uint32_t v = p[off_x + x + y * box.w];

                    // if more than half solid - mark solid and advance to the next bit
                    if (((v & 0xff000000) >> 24) > 128) *dp |= 1 << bit; // not 7 - bit because little endian

                    // same as & 0b1000 - just checks if it got to 8 (totally necessary)
                    if (++bit & 8)
                    {
                        bit = 0;
                        ++dp;
                    }
                }
            }

            // start the next box from a new byte
            if (bit)
            {
                bit = 0;
                ++dp;
            }
        }
    }

    mse::FileIO::write(mask_path, { data, total_size });
    delete[] data;
}
