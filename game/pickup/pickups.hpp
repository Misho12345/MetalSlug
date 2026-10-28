#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class Drop : uint8_t
    {
        RifleSpray,
        RifleFireball,
        Points,
        COUNT
    };

    class Pickups final
    {
    public:
        static constexpr uint32_t DROP_POINTS = 500;

        mse::entity_id spawn_crate(mse::Scene& scene, mse::ivec2 pos) const;

        // pos is the floor under the middle of it, drops have no gravity
        void drop(mse::Scene& scene, mse::ivec2 pos, Drop what) const;
        void drop_random(mse::Scene& scene, mse::ivec2 pos) const;

        // the crates' callback only flags them and this does the rest after the collision step,
        // creating entities in the middle of it can move the pools it's iterating
        void update(mse::Scene& scene) const;
    };
}
