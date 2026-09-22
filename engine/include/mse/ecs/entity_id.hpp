#pragma once
#include "mse/pch.hpp"

namespace mse
{
    /**
     * @brief A unique identifier for an entity
     * @details This is a lightweight handle for an entity in the scene.
     * It has a version number that is incremented when the entity is destroyed and recreated to prevent dangling references.
     */
    struct entity_id final
    {
        uint32_t value;

        static constexpr uint32_t IDX_BITS = 24;
        static constexpr uint32_t IDX_MASK = (1 << IDX_BITS) - 1;

        constexpr entity_id(const uint32_t idx, const uint32_t version)
            : value{ (idx & IDX_MASK) | (version << IDX_BITS) } {}

        constexpr entity_id() : value{ IDX_MASK } {}

        [[nodiscard]] constexpr uint32_t idx() const { return value & IDX_MASK; }
        [[nodiscard]] constexpr uint8_t version() const { return static_cast<uint8_t>(value >> IDX_BITS); }

        constexpr bool operator==(const entity_id other) const { return value == other.value; }

        operator bool() const { return value != IDX_MASK; }
    };
}