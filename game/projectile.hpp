#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class ProjectileType : uint8_t { Bullet, Fireball, Grenade };

    // on everything on the PlayerProjectile and EnemyProjectile layers, so what gets hit knows by what
    struct Projectile final
    {
        ProjectileType type;
    };
}
