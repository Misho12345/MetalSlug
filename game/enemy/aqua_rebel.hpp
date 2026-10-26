#pragma once
#include "enemy.hpp"

namespace ms
{
    enum class AquaRebelState : uint8_t
    {
        Idle,  // in the water, only the head out
        Jump,  // up to about the player's height
        Throw, // hangs at the top for the throw, then drops back in
        Submerge,
        Dead,
        COUNT
    };

    class AquaRebelManager final : public EnemyManager
    {
    public:
        void register_machine() const override;

        // pos is the water's surface
        mse::entity_id spawn(mse::Scene& scene, mse::ivec2 pos) const override;
    };
}
