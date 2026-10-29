#pragma once

#include "player/player.hpp"
#include "pow/pow.hpp"
#include "pickup/pickups.hpp"
#include "enemy/enemies.hpp"
#include "boss/boss.hpp"
#include "hud/hud.hpp"
#include "hud/debrief.hpp"
#include "sound.hpp"

namespace ms
{
    struct UserCtx
    {
        Player player;

        POWManager pows;
        Pickups    pickups;
        Enemies    enemies;
        Boss       boss;

        Hud     hud;
        Debrief debrief;

        Sounds sounds;
    };
}
