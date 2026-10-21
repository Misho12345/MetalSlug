#pragma once
#include "components.hpp"

namespace ms
{
    constexpr float JUMP_HEIGHT    = 56.0f;
    constexpr float JUMP_APEX_TIME = 0.36f; // from takeoff to the highest point

    constexpr float JUMP_GRAVITY  = 2.0f * JUMP_HEIGHT / (JUMP_APEX_TIME * JUMP_APEX_TIME);
    constexpr float JUMP_VELOCITY = 2.0f * JUMP_HEIGHT / JUMP_APEX_TIME;

    constexpr float WALK_SPEED  = 100.0f;
    constexpr float CRAWL_SPEED = 50.0f;

    const LegsSM& legs_sm();
}
