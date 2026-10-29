#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class ProjectileType : uint8_t { Bullet, Fireball, Grenade, Missile, CannonShot };

    // on everything on the PlayerProjectile and EnemyProjectile layers, so what gets hit knows by what
    struct Projectile final
    {
        ProjectileType type;
    };
}
