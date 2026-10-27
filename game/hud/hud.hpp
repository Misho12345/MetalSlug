#pragma once
#include "number.hpp"

namespace ms
{
    class Hud final
    {
    public:
        void create(mse::Scene& scene);
        void update(mse::Scene& scene) const;

    private:
        NumberDisplay score_;
        NumberDisplay ammo_;
        NumberDisplay bombs_;
        NumberDisplay lives_;

        mse::entity_id inf_;
    };
}
