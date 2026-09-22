#include "mse/pch.hpp"
#include "tile_map.hpp"

namespace mse
{
    bool TileMap::load(const string_view path)
    {
        const string csv = FileIO::read(path);
        if (csv.empty()) return false;

        int width = 0, height = 0;

        for (const char* c = csv.data(); *c; ++c)
        {
            if (*c == '\r') continue;
            if (*c == '\n') ++height;
            else if (!height && *c == ',') ++width;
        }

        ++width; // n commas -> n + 1 numbers

        size_ = { width, height };
        data_.resize(width * height, TileType::Air);

        // -1 = air
        //  0 = floor
        // ...
        size_t i = 0;
        for (const char* c = csv.data(); *c; ++c)
        {
            if (*c == '\n' || *c == '\r' || *c == ',') continue;

            if (*c == '-') // skip -1,
            {
                ++c;
                ++i;
                continue;
            }

            // because I don't plan on having more than 10 non-air tile types, I won't use atoi but just assume it's 1 digit
            switch (*c - '0')
            {
                case 0: data_[i] = TileType::Floor; break;
                default: assert(false && "not implemented"); break;
            }

            ++i;
        }

        return true;
    }
}
