#pragma once
#include "components.hpp"

namespace player
{
    const BodySM& body_sm();

    // turning and crouching start in the legs, they hand the body its sprite through this
    void begin_transition(Scene& scene, entity_id body, Pose pose);
}
