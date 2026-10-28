#pragma once
#include "mse/mse.hpp"

namespace ms
{
    // after create_bg, the markers are relative to its top left like the tile map
    void load_props(mse::Scene& scene);

    // after the camera moved, hides the far layer outside its stretch
    void update_props(mse::Scene& scene);
}
