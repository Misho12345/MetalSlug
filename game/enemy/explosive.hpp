#pragma once
#include "mse/mse.hpp"

namespace ms
{
    // grenades and missiles, they go off on the first thing they hit
    struct Explosive final
    {
        bool exploded{ false };
    };

    // the collider callback, it only marks it, the projectile's own machine calls explode() the tick after
    void on_explosive_hit(mse::Scene& scene, mse::entity_id self, mse::entity_id, uint32_t);

    // a small explosion standing on at, destroys e
    void explode(mse::Scene& scene, mse::entity_id e, mse::ivec2 at);
}
