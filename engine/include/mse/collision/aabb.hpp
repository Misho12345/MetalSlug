#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct aabb final
    {
        glm::ivec2 min;
        glm::ivec2 max;

        constexpr bool operator&(const aabb& other) const
        {
            return min.x <= other.max.x && min.y <= other.max.y &&
                    max.x >= other.min.x && max.y >= other.min.y;
        }

        constexpr operator bool() const { return min.x <= max.x && min.y <= max.y; }

        [[nodiscard]]
        static constexpr aabb overlap(const aabb& a, const aabb& b)
        {
            return aabb{ glm::max(a.min, b.min), glm::min(a.max, b.max) };
        }

        [[nodiscard]]
        constexpr bool contains_excl(const glm::ivec2 point) const
        {
            return point.x > min.x && point.x < max.x &&
                    point.y > min.y && point.y < max.y;
        }

        [[nodiscard]]
        constexpr bool contains(const glm::ivec2 point) const
        {
            return point.x >= min.x && point.x <= max.x &&
                    point.y >= min.y && point.y <= max.y;
        }

        [[nodiscard]] glm::ivec2 center() const { return (min + max) / 2; }
        [[nodiscard]] glm::ivec2 size() const { return max - min; }



        [[nodiscard]] aabb operator+(const glm::ivec2 rhs) const { return { min + rhs, max + rhs }; }
        [[nodiscard]] aabb operator-(const glm::ivec2 rhs) const { return { min - rhs, max - rhs }; }
        [[nodiscard]] aabb operator*(const glm::ivec2 rhs) const { return { min * rhs, max * rhs }; }
        [[nodiscard]] aabb operator/(const glm::ivec2 rhs) const { return { min / rhs, max / rhs }; }

        aabb& operator+=(const glm::ivec2 rhs)
        {
            min += rhs;
            max += rhs;
            return *this;
        }

        aabb& operator-=(const glm::ivec2 rhs)
        {
            min -= rhs;
            max -= rhs;
            return *this;
        }

        aabb& operator*=(const glm::ivec2 rhs)
        {
            min *= rhs;
            max *= rhs;
            return *this;
        }

        aabb& operator/=(const glm::ivec2 rhs)
        {
            min /= rhs;
            max /= rhs;
            return *this;
        }
    };
}
