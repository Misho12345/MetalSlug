#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class BossState : uint8_t
    {
        Asleep, // turned away, until the player gets close
        Wake,   // the camera pans over and locks
        Deploy, // turns to face the player
        Open,
        Fire,
        Close,
        Move, // turns away and back, coming out at the other height
        Dying,
        COUNT
    };

    // Tetsuyuki, the mission 1 boss. The ship is scenery, the cannon on its hull is what gets shot
    class Boss final
    {
    public:
        void register_machines() const;

        // front is the top left of waterfall-front, the whole arena is placed from it
        void create(mse::Scene& scene, mse::ivec2 front);

        // after the player's moved, keeps him off the hull and on the locked screen
        void late_update(mse::Scene& scene) const;

        // where the camera's center goes when it wants to be at follow: kept on the map, lifted in the arena,
        // panned over to the boss once he's awake
        mse::ivec2 camera(const mse::Scene& scene, mse::ivec2 follow) const;

        bool defeated(const mse::Scene& scene) const;

    private:
        // where the camera's center may go. The map, plus the sky over the arena
        mse::aabb camera_limits(const mse::Scene& scene) const;

        // invalid until a map with the arena is loaded, and again once a restart destroyed it
        mse::entity_id cannon_;
        mse::ivec2     front_{};
    };
}
