#pragma once
#include "weapons.hpp"

namespace ms
{
    void create_bullet_pool(mse::Scene& scene);

    void fire_bullet(mse::Scene& scene, WeaponType type, mse::ivec2 from, mse::vec2 velocity, int8_t facing);

    // fireball animations, and free when leaving the screen
    void update_bullets(mse::Scene& scene);
}
