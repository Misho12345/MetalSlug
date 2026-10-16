#include "common/common.hpp"

#include <algorithm>
#include <cstring>
#include <cassert>

#include <stb_image.h>
#include <nlohmann/json.hpp>

#include "atlas.hpp"

using nlohmann::json;

// args format:
// <sprite_idx_0> <anim_count_0> <animation_0_0> <frame_count_0_0>, ... <animation_0_N> <frame_count_0_N>,
// <sprite_idx_1> <anim_count_1> <animation_1_0> <frame_count_1_0>, ... <animation_1_N> <frame_count_1_N>,
// ...
// --out <out_dir>
static void validate_args(
    [[maybe_unused]] const int argc,
    [[maybe_unused]] const char** argv)
{
    assert(argc >= 3);
    assert(strcmp(argv[argc - 2], "--out") == 0);
    // TODO: (not important) maybe make a proper validation
}

// where the next box goes: right after the last one, on a shelf as tall as the first (tallest) box placed on it
struct Cursor
{
    uint32_t x, y, shelf_h;
};

// frames [0, first_row) start at (x, y); the rest wrap onto rows of their own,
// per_row frames each, starting at (0, y + box.h) and box.h apart.
struct Placement
{
    uint32_t x, y;
    uint32_t first_row, per_row;
};

static bool place(const Box& box, Cursor& cursor, Placement& out)
{
    const uint32_t frame_w = box.w / box.frame_count;
    assert(frame_w <= Atlas::SIZE && "a single frame is wider than the atlas");

    const uint32_t per_row = Atlas::SIZE / frame_w;

    // continue on the current shelf if at least one frame still fits on it
    Cursor c = cursor;
    if (c.x + frame_w > Atlas::SIZE)
    {
        c.x = 0;
        c.y += c.shelf_h;
        c.shelf_h = box.h;
    }

    const uint32_t first_row = std::min(box.frame_count, (Atlas::SIZE - c.x) / frame_w);
    const uint32_t rest      = box.frame_count - first_row;
    const uint32_t rows      = (rest + per_row - 1) / per_row;

    // a strip that wraps sits on the bottom of its shelf, so its next row starts
    // right under its own first one. the shelf can be taller than the strip
    const uint32_t y      = rows ? c.y + c.shelf_h - box.h : c.y;
    const uint32_t next_y = y + box.h;
    const uint32_t bottom = next_y + rows * box.h;
    if (bottom > Atlas::SIZE) return false;

    out = { c.x, y, first_row, per_row };

    if (rows)
    {
        // the last wrapped row is now the current shelf, and it's this box's height
        const uint32_t last_row_frames = rest - (rows - 1) * per_row;
        cursor = { last_row_frames * frame_w, next_y + (rows - 1) * box.h, box.h };
    }
    else
    {
        cursor = { c.x + first_row * frame_w, c.y, c.shelf_h };
    }

    return true;
}

int main(const int argc, const char** argv)
{
    validate_args(argc, argv);

    // get the number of actual image files to be packed into atlases
    size_t files_count = (size_t)(argc - 3);
    for (int i = 2; i < argc - 2; )
    {
        files_count -= 2;
        i += std::stoi(argv[i]) * 2 + 2;
    }

    // each animation path has it's own frame count so the parameters are x2 the file count
    files_count /= 2;

    if (files_count == 0)
    {
        printf("no files to pack\n");
        return 0;
    }

    mse::vector<Box>   boxes;
    mse::vector<Box*>  box_ptrs;
    mse::vector<Atlas> atlases;

    // reserving ensures no reallocation, so storing pointers in box_ptrs is fine
    boxes.reserve(files_count);
    box_ptrs.reserve(files_count);
    atlases.reserve(5);

    for (size_t i = 1; i < (size_t)(argc - 3); )
    {
        uint32_t sprite_id = (uint32_t)std::stoi(argv[i]);
        const uint32_t num_anim = (uint32_t)std::stoi(argv[i + 1]);

        uint32_t j = 0;

        for (; j < num_anim; ++j)
        {
            boxes.emplace_back(
                argv[i + j * 2u + 2u],
                sprite_id,
                j,
                (uint32_t)std::stoi(argv[i + j * 2u + 3u]));

            box_ptrs.emplace_back(&boxes.back());
        }

        i += j * 2 + 2;
    }

    qsort(box_ptrs.data(), files_count, sizeof(Box*), [](const void* a, const void* b)
    {
        const Box* box_a = *(const Box* const*)a;
        const Box* box_b = *(const Box* const*)b;
        return (int)box_b->h - (int)box_a->h; // descending by height
    });

    atlases.emplace_back();

    Cursor cursor{ 0, 0, box_ptrs[0]->h };

    json j;
    j["atlases"] = json::array();
    j["atlases"].push_back(json::array());

    // TODO: make a more sophisticated algorithm in the future, have the option to rotate horizontally
    for (const Box* box : box_ptrs)
    {
        Placement at;
        if (!place(*box, cursor, at))
        {
            atlases.emplace_back();
            j["atlases"].push_back(json::array());
            cursor = { 0, 0, box->h };

            [[maybe_unused]] const bool fits = place(*box, cursor, at);
            assert(fits && "animation strip doesn't fit in an empty atlas");
        }

        const uint32_t frame_w = box->w / box->frame_count;

        atlases.back().add(box, 0, at.first_row * frame_w, at.x, at.y);

        uint32_t row_y = at.y + box->h;
        for (uint32_t f = at.first_row; f < box->frame_count; f += at.per_row, row_y += box->h)
        {
            const uint32_t count = std::min(at.per_row, box->frame_count - f);
            atlases.back().add(box, f * frame_w, count * frame_w, 0, row_y);
        }

        j["atlases"][atlases.size() - 1].push_back({
            { "sprite_id", box->sprite_id },
            { "anim_id", box->anim_id },
            { "x", at.x },
            { "y", at.y },
            { "w", box->w },
            { "h", box->h }
        });
    }


    char path[256];
    size_t i = 0;
    for (const Atlas& atlas : atlases)
    {
        snprintf(path, sizeof(path), "%s/atlas_%zu.png", argv[argc - 1], i++);
        printf("saving %s\n", path);
        atlas.save(path);
    }

    snprintf(path, sizeof(path), "%s/mask", argv[argc - 1]);

    char path_flipped[256];
    snprintf(path_flipped, sizeof(path_flipped), "%s/mask_flipped", argv[argc - 1]);

    printf("saving %s and %s\n", path, path_flipped);

    // using the original box order so the masks order matchers what anim.hpp will provide
    save_masks(boxes, path, path_flipped);

    snprintf(path, sizeof(path), "%s/meta.json", argv[argc - 1]);
    printf("saving %s\n", path);
    mse::FileIO::write(path, j.dump(2).c_str());
}