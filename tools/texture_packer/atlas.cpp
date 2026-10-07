#include "atlas.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

#include <stb_image.h>
#include <stb_image_write.h>


Box::Box(const char* _image_path, const uint32_t _sprite_id, const uint32_t _anim_id, const uint32_t _frame_count)
    : image_path{ _image_path },
      sprite_id{ _sprite_id },
      anim_id{ _anim_id },
      frame_count{ _frame_count }
{
    int w_, h_, t;
    data = (uint32_t*)(stbi_load(image_path, &w_, &h_, &t, 4));
    w    = (uint32_t)w_;
    h    = (uint32_t)h_;
}

Box::~Box() { if (data) stbi_image_free(data); }

Box::Box(Box&& other) noexcept
    : image_path{ std::exchange(other.image_path, nullptr) },
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
    mse::vector<uint32_t> data(SIZE * SIZE);

    // save to texture atlas
    for (const Box* box : boxes_)
    {
        const uint32_t* p = box->data;

        if (box->rotated)
        {
            // copy pixel by pixel because it's rotated
            for (uint32_t x = 0; x < box->h; ++x)
            {
                for (uint32_t y = 0; y < box->w; ++y, ++p)
                {
                    data[(box->x + x + (box->y + y) * SIZE)] = *p;
                }
            }
        }
        else
        {
            // copy row by row
            for (uint32_t y = 0; y < box->h; ++y, p += box->w)
            {
                memcpy(&data[box->x + (box->y + y) * SIZE], p, box->w * sizeof(uint32_t));
            }
        }
    }

    stbi_write_png(atlas_path, SIZE, SIZE, sizeof(uint32_t), data.data(), SIZE * sizeof(uint32_t));
}

void save_masks(const mse::span<const Box> boxes, const char* mask_path, const char* flipped_mask_path)
{
    size_t total_size = 0;

    for (const Box& box : boxes)
    {
        assert(box.w % box.frame_count == 0 && "Image isn't evenly divisible by the frame count");
        // round up to the count of bytes
        total_size += (box.w / box.frame_count * box.h + 7) / 8 * box.frame_count;
    }

    mse::vector<uint8_t> data(total_size);
    mse::vector<uint8_t> data_flipped(total_size);

    uint8_t* dp = data.data();
    uint8_t* fdp = data_flipped.data();

    uint32_t bit = 0;

    for (const Box& box : boxes)
    {
        // for each box go through each frame and put the data of the frames sequentially
        // that way when loaded each frame will have a pointer for the it's mask, and not have to find
        // the parts of the mask for the specific frame in the entire animation data block

        const uint32_t* p       = box.data;
        const uint32_t  frame_w = box.w / box.frame_count;

        for (uint32_t f = 0; f < box.frame_count; ++f)
        {
            const size_t off_x = frame_w * f;
            for (uint32_t y = 0; y < box.h; ++y)
            {
                for (uint32_t x = 0; x < frame_w; ++x)
                {
                   const uint32_t v = p[off_x + x + y * box.w];
                   const uint32_t fv = p[off_x + frame_w - x - 1 + y * box.w];

                   // if more than half solid - mark solid and advance to the next bit
                   if (((v & 0xff000000) >> 24) > 128) *dp |= 1 << bit; // not (7 - bit) because little endian
                   if (((fv & 0xff000000) >> 24) > 128) *fdp |= 1 << bit;

                   if (++bit >= 8)
                   {
                       bit = 0;
                       ++dp;
                       ++fdp;
                   }
                }
            }

            // start the next box from a new byte
            if (bit)
            {
                bit = 0;
                ++dp;
                ++fdp;
            }
        }
    }

    mse::FileIO::write(mask_path, data);
    mse::FileIO::write(flipped_mask_path, data_flipped);
}
