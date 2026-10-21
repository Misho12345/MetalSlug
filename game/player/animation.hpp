#pragma once
#include "components.hpp"

namespace ms
{
    // picks the pose from both states and the weapon
    void animate_body(Scene& scene, entity_id legs, entity_id body);

    // lines up the sprites and fits the colliders to them
    ivec2 place_sprites(Scene& scene, entity_id legs, entity_id body);
}
