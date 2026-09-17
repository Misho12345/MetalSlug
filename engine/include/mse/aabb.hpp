#pragma once
#include "pch.hpp"

namespace mse
{
    struct aabb final
    {
        glm::vec2 min;
        glm::vec2 max;

        constexpr bool operator&(const aabb& other) const
        {
            return min.x <= other.max.x && min.y <= other.max.y &&
                   max.x >= other.min.x && max.y >= other.min.y;
        }

        constexpr operator bool() const { return min.x <= max.x && min.y <= max.y; }

        static constexpr aabb overlap(const aabb& a, const aabb& b)
        {
            return aabb{ glm::max(a.min, b.min), glm::min(a.max, b.max) };
        }

        constexpr bool contains_excl(const glm::vec2 point) const
        {
            return point.x > min.x && point.x < max.x &&
                   point.y > min.y && point.y < max.y;
        }

        constexpr bool contains(const glm::vec2 point) const
        {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y;
        }

        glm::vec2 center() const { return (min + max) / 2.0f; }
        glm::vec2 size() const { return max - min; }
    };
}
