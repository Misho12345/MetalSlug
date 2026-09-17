#include "common/common.hpp"

#include <cstring>
#include <cassert>
#include <cstdio>

#include <stb_image.h>
#include <nlohmann/json.hpp>

#include "atlas.hpp"

using nlohmann::json;

// args format:
// <sprite_idx_0> <anim_count_0> <animation_0_0> <frame_count_0_0>, ... <animation_0_N> <frame_count_0_N>,
// <sprite_idx_1> <anim_count_1> <animation_1_0> <frame_count_1_0>, ... <animation_1_N> <frame_count_1_N>,
// ...
// --out <out_dir>
static void validate_args(const int argc, const char** argv)
{
    assert(argc >= 3);
    assert(strcmp(argv[argc - 2], "--out") == 0);
    // TODO: (not important) maybe make a proper validation
}

int main(const int argc, const char** argv)
{
    validate_args(argc, argv);

    // get the number of actual image files to be packed into atlases
    size_t files_count = argc - 3;
    for (size_t i = 2; i < argc - 2; )
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

    for (size_t i = 1; i < argc - 3; )
    {
        int sprite_id = std::stoi(argv[i]);
        int num_anim = std::stoi(argv[i + 1]);

        int j = 0;

        for (; j < num_anim; ++j)
        {
            boxes.emplace_back(argv[i + j * 2 + 2], sprite_id, j, std::stoi(argv[i + j * 2 + 3]));
            box_ptrs.emplace_back(&boxes.back());
        }

        i += j * 2 + 2;
    }

    qsort(box_ptrs.data(), files_count, sizeof(Box*), [](const void* a, const void* b)
    {
        const Box* box_a = *static_cast<const Box* const*>(a);
        const Box* box_b = *static_cast<const Box* const*>(b);
        return box_b->h - box_a->h; // descending by height
    });

    atlases.emplace_back();

    int  x = 0, y = 0, h_off = box_ptrs[0]->h;

    json j;
    j["atlases"] = json::array();
    j["atlases"].push_back(json::array());

    // TODO: make a more sophisticated algorithm in the future, have the option to rotate horizontally
    for (Box* box : box_ptrs)
    {
        if (x + box->w > Atlas::SIZE)
        {
            x = 0;
            y += h_off;
            h_off = box->h;
        }

        if (y + box->h > Atlas::SIZE)
        {
            atlases.emplace_back();
            j["atlases"].push_back(json::array());
            x = y = 0;
            h_off = box->h;
        }

        box->x = x;
        box->y = y;

        x += box->w;

        atlases.back().add(box);

        j["atlases"][atlases.size() - 1].push_back({
            { "sprite_id", box->sprite_id },
            { "anim_id", box->anim_id },
            { "frame_count", box->frame_count },
            { "x", box->x },
            { "y", box->y },
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

    snprintf(path, sizeof(path), "%s/meta.json", argv[argc - 1]);

    mse::FileIO::write(path, j.dump(2).c_str());
}
