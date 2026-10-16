#pragma once
#include "mse/mse.hpp"

namespace player
{
    class Player final
    {
    public:
        void create(mse::Scene& scene);

        static void register_machines();

        void update(mse::Scene& scene) const;
        void late_update(mse::Scene& scene) const;

    private:
        mse::entity_id legs_;
        mse::entity_id body_;
    };
}
