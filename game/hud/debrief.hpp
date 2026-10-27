#pragma once
#include "mse/mse.hpp"

namespace ms
{
    // the "RECAPTURED PRISONER" panel from the original, over the level which fades a bit darker
    class Debrief final
    {
    public:
        void create(mse::Scene& scene);

        // dims the level, then puts the panel up, pays out the POWs and keeps the one on the panel waving
        void update(mse::Scene& scene, uint16_t ticks);

        // carries on by itself once there's been time to read it
        static bool done(uint16_t ticks);

    private:
        void show(mse::Scene& scene);

        mse::entity_id dim_;
        mse::entity_id pow_;
    };
}
