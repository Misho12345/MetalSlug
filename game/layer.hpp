#pragma once
#include "mse/mse.hpp"

enum class Layer : uint32_t
{
    Player = 1 << 0,
    Enemy  = 1 << 1,

    PlayerBullet = 1 << 2,
    EnemyBullet  = 1 << 3,

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
