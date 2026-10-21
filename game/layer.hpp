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

        PlayerBullet = 1 << 4,
        EnemyBullet  = 1 << 5,

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
