#pragma once

#include "vfx/vfx.hpp"
#include "player/player.hpp"
#include "pow/pow.hpp"
#include "pickup/pickups.hpp"
#include "enemy/enemies.hpp"
#include "hud/hud.hpp"
#include "hud/debrief.hpp"
#include "sound.hpp"

namespace ms
{
    struct UserCtx
    {
        Vfx    vfx;
        Player player;

        POWManager pows;
        Pickups    pickups;
        Enemies    enemies;

        Hud     hud;
        Debrief debrief;

        Sounds sounds;
    };
}
