#include "common/common.hpp"

#include <cstring>
#include <cassert>
#include <cstdio>

#include <stb_image.h>
#include <nlohmann/json.hpp>

#include "atlas.hpp"

using nlohmann::json;

void validate_args(const int argc, const char** argv)
{
    assert(argc >= 3);
    assert(strcmp(argv[argc - 2], "--out") == 0);
    // TODO: (not important) maybe make a proper validation
}

int main(const int argc, const char** argv)
{
    validate_args(argc, argv);

    size_t files_count = argc - 3;
    for (size_t i = 2; i < argc - 2; )
    {
        files_count -= 2;
        i += std::stoi(argv[i]) + 2;
    }

    mse::vector<Box>   boxes;
    mse::vector<Box*>  box_ptrs;
    mse::vector<Atlas> atlases;

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
            boxes.emplace_back(argv[i + j + 2], sprite_id, j);
            box_ptrs.emplace_back(&boxes.back());
        }

        i += j + 2;
    }

    qsort(box_ptrs.data(), files_count, sizeof(Box*), [](const void* a, const void* b)
    {
        const Box* box_a = *static_cast<const Box* const*>(a);
        const Box* box_b = *static_cast<const Box* const*>(b);
        return box_a->h - box_b->h;
    });

    atlases.emplace_back();

    int  box_idx = static_cast<int>(box_ptrs.size()) - 1;
    int  x = 0, y = 0, h_off = box_idx >= 0 ? box_ptrs[box_idx]->h : 0;

    json j;
    j["atlases"] = json::array();
    j["atlases"].push_back(json::array());

    // TODO: make a more sophisticated algorithm in the future, have the option to rotate horizontally
    while (box_idx >= 0)
    {
        Box& box = *box_ptrs[box_idx];

        if (x + box.w > Atlas::SIZE)
        {
            x = 0;
            y += h_off;
            h_off = box.h;
        }

        if (y + box.h > Atlas::SIZE)
        {
            atlases.emplace_back();
            j["atlases"].push_back(json::array());
            x = y = 0;
            h_off = box.h;
        }

        box.x = x;
        box.y = y;

        x += box.w;

        atlases.back().add(&box);

        j["atlases"][atlases.size() - 1].push_back({
            { "sprite_id", box.sprite_id },
            { "anim_id", box.anim_id },
            { "x", box.x },
            { "y", box.y },
            { "w", box.w },
            { "h", box.h }
        });

        --box_idx;
    }


    char path[256];

    for (size_t i = 0; i < atlases.size(); ++i)
    {
        snprintf(path, sizeof(path), "%s/atlas_%zu.png", argv[argc - 1], i);
        printf("saving %s\n", path);
        atlases[i].save(path);
    }

    snprintf(path, sizeof(path), "%s/meta.json", argv[argc - 1]);

    mse::FileIO::write(path, j.dump(2).c_str());
}
