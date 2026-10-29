#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class BossState : uint8_t
    {
        Asleep, // before the boss battle, till the player gets close
        Wake,   // the camera pans over and locks
        Deploy, // turns to face the player
        Open,
        Fire,
        Close,
        Move, // turns away and back, coming out at the other height
        Dying,
        COUNT
    };

    class Boss final
    {
    public:
        void register_machines() const;

        void create(mse::Scene& scene, mse::ivec2 front);
        void late_update(mse::Scene& scene) const;

        mse::ivec2 camera(const mse::Scene& scene, mse::ivec2 follow) const;

        bool defeated(const mse::Scene& scene) const;

    private:
        mse::aabb camera_limits(const mse::Scene& scene) const;

        mse::entity_id cannon_;
        mse::ivec2     front_{};
    };
}
