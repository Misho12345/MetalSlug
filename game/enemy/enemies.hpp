#pragma once
#include "rebel.hpp"
#include "aqua_rebel.hpp"

namespace ms
{
    enum class EnemyKind : uint8_t
    {
        Rebel,
        AquaRebel,
        COUNT
    };

    class Enemies final
    {
    public:
        void register_machines() const;

        mse::entity_id spawn(mse::Scene& scene, mse::ivec2 pos, EnemyKind kind) const;

    private:
        const EnemyManager& type(EnemyKind kind) const;

        RebelManager     rebel_;
        AquaRebelManager aqua_rebel_;
    };
}
