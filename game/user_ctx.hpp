#pragma once

#include "vfx/vfx.hpp"
#include "player/player.hpp"
#include "pow/pow.hpp"
#include "enemy/rebel.hpp"

namespace ms
{
    struct UserCtx
    {
        Vfx    vfx;
        Player player;

        POWManager   pows;
        RebelManager rebels;
    };
}
