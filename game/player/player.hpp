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

        uint32_t score() const { return score_; }
        void     add_score(const uint32_t points) { score_ += points; }

        uint32_t pows_freed() const { return pows_freed_; }
        void     free_pow() { ++pows_freed_; }

    private:
        mse::entity_id legs_;
        mse::entity_id body_;

        // here and not in BodyData, the body is destroyed on the way to the game over screen
        uint32_t score_{};
        uint32_t pows_freed_{};
    };
}
