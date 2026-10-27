#pragma once

#include "vfx/vfx.hpp"
#include "player/player.hpp"
#include "pow/pow.hpp"
#include "enemy/enemies.hpp"
#include "hud/hud.hpp"
#include "hud/debrief.hpp"

namespace ms
{
    struct UserCtx
    {
        Vfx    vfx;
        Player player;

        POWManager pows;
        Enemies    enemies;

        Hud     hud;
        Debrief debrief;
    };
}
