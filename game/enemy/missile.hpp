#pragma once
#include "mse/mse.hpp"

namespace ms
{
    void register_missile_machine();

    // dir is -1 or 1, it drifts to target_y over the way to target_x, explodes on the tile map or the player
    void launch_missile(mse::Scene& scene, mse::ivec2 from, int8_t dir, mse::ivec2 target);
}
