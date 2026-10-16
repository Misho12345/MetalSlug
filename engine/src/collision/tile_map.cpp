#include "mse/pch.hpp"
#include "mse/collision/tile_map.hpp"

namespace mse
{
    bool TileMap::load(const string_view path)
    {
        const Csv csv = FileIO::read<Csv>(path);
        if (csv.data.empty()) return false;

        size_ = { csv.width, csv.height };
        data_.resize(csv.data.size());

        const int32_t* cd = csv.data.data();
        TileType* td = data_.data();

        for (uint32_t y = 0; y < csv.height; ++y)
        {
            for (uint32_t x = 0; x < csv.width; ++x, ++cd, ++td)
            {
                switch (*cd)
                {
                    case -1: break;
                    case 0: *td = TileType::Floor; break;
                    case 1: *td = TileType::Platform; break;
                    case 2: *td = TileType::SlopeL; break;
                    case 3: *td = TileType::SlopeR; break;
                    default: assert(!"not implemented"); return false;
                }
            }
        }

        return true;
    }
}
