#pragma once
#include "mse/mse.hpp"

namespace ms
{
    // moves e so the bottom middle of its sprite is at to, returns the shift
    inline mse::ivec2 align(mse::Scene& scene, const mse::entity_id e, const mse::ivec2 to)
    {
        mse::Transform& tr = scene.get<mse::Transform>(e);
        const mse::aabb b  = scene.get<mse::Sprite>(e).bounds(tr);

        const mse::ivec2 shift = to - mse::ivec2{ tr.position.x, b.max.y };
        tr.position += shift;
        return shift;
    }

    // what everything placed on the spawn layer starts as, standing on pos
    template <mse::anim::sprite_enum E>
    mse::entity_id spawn_standing(mse::Scene& scene, const mse::ivec2 pos, const E idle)
    {
        const mse::entity_id e = scene.create_entity();

        scene.set<mse::Transform>(e);
        scene.set<mse::Sprite>(e, idle, mse::should_loop::yes);
        align(scene, e, pos);

        return e;
    }
}
