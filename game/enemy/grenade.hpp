#pragma once
#include "mse/mse.hpp"

namespace ms
{
    void register_grenade_machine();

    // thrown so it comes down around target, explodes on the tile map or the player
    void launch_grenade(mse::Scene& scene, mse::ivec2 from, mse::ivec2 target);

    uint32_t live_grenades(mse::Scene& scene);
}
