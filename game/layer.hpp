#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class Layer : uint32_t
    {
        Player = 1 << 0,
        Enemy  = 1 << 1,

        POW = 1 << 2,

        WeaponUpgrade = 1 << 3,

        PlayerProjectile = 1 << 4,
        EnemyProjectile  = 1 << 5,

        EnemyKnife = 1 << 6,

        Crate        = 1 << 7,
        PointsPickup = 1 << 8, // not in the player's target_layer, unlike WeaponUpgrade the pickup scores itself

        TileMap = mse::Collider::TILE_MAP_LAYER
    };

    inline uint32_t operator&(const Layer lhs, const Layer rhs) { return (uint32_t)lhs & (uint32_t)rhs; }
    inline uint32_t operator&(const Layer lhs, const uint32_t rhs) { return (uint32_t)lhs & rhs; }
    inline uint32_t operator&(const uint32_t lhs, const Layer rhs) { return lhs & (uint32_t)rhs; }
    inline uint32_t operator&=(uint32_t& lhs, const Layer rhs) { return lhs &= (uint32_t)rhs; }

    inline uint32_t operator|(const Layer lhs, const Layer rhs) { return (uint32_t)lhs | (uint32_t)rhs; }
    inline uint32_t operator|(const Layer lhs, const uint32_t rhs) { return (uint32_t)lhs | rhs; }
    inline uint32_t operator|(const uint32_t lhs, const Layer rhs) { return lhs | (uint32_t)rhs; }
    inline uint32_t operator|=(uint32_t& lhs, const Layer rhs) { return lhs |= (uint32_t)rhs; }
}
