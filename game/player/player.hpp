#pragma once
#include "mse/mse.hpp"

namespace ms
{
    class Player final
    {
    public:
        void create(mse::Scene& scene);

        void register_machines() const;

        void update(mse::Scene& scene) const;
        void late_update(mse::Scene& scene) const;

        mse::entity_id legs() const { return legs_; }
        mse::entity_id body() const { return body_; }

        bool   alive(const mse::Scene& scene) const;
        bool   shooting(const mse::Scene& scene) const;
        int8_t facing(const mse::Scene& scene) const;
        bool   out_of_lives(const mse::Scene& scene) const;

    private:
        mse::entity_id legs_;
        mse::entity_id body_;
    };
}
