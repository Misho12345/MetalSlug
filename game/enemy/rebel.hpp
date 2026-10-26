#pragma once
#include "enemy.hpp"

namespace ms
{
    enum class RebelState : uint8_t
    {
        // only until it first sees the player
        Idle,

        // one shot animations between the others: turning, getting scared, stopping
        Transition,

        RunAway,
        Sneak,
        Attack,
        Wander, // walking around: the player is dead or right above it, or a wall too high to jump is in the way

        // jumping off a ledge when attacking, falling otherwise
        Air,

        Slash,
        Throw,

        Dying, // burning
        Dead,

        COUNT
    };

    class RebelManager final : public EnemyManager
    {
    public:
        void register_machine() const override;

        mse::entity_id spawn(mse::Scene& scene, mse::ivec2 pos) const override;
    };
}
