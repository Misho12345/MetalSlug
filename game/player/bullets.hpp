#pragma once
#include "weapons.hpp"

namespace player
{
    void create_bullet_pool(Scene& scene);

    void fire_bullet(Scene& scene, WeaponType type, ivec2 from, vec2 velocity, int8_t facing);

    // fireball animations, and free when leaving the screen
    void update_bullets(Scene& scene);
}
