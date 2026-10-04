#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct aabb final
    {
        static aabb screen;

        ivec2 min;
        ivec2 max;

        /**
         * @brief access element in aabb
         * @param v idx of element (0 or 1)
         * @return min for v == 0; max for v == 1
         */
        constexpr ivec2 operator[](const int v) const
        {
            assert(v == 0 || v == 1 && "v must be 0 or 1");
            return v ? max : min;
        }

        constexpr bool operator&(const aabb& other) const
        {
            return min.x < other.max.x && min.y < other.max.y &&
                    max.x > other.min.x && max.y > other.min.y;
        }

        constexpr aabb& operator|=(const aabb& other)
        {
            if (&other == this) return *this;

            min = glm::min(min, other.min);
            max = glm::max(max, other.max);

            return *this;
        }

        constexpr operator bool() const { return min.x < max.x && min.y < max.y; }

        [[nodiscard]]
        static constexpr aabb overlap(const aabb& a, const aabb& b)
        {
            return aabb{ glm::max(a.min, b.min), glm::min(a.max, b.max) };
        }

        [[nodiscard]]
        constexpr bool contains_excl(const ivec2 point) const
        {
            return point.x > min.x && point.x < max.x &&
                    point.y > min.y && point.y < max.y;
        }

        [[nodiscard]]
        constexpr bool contains(const ivec2 point) const
        {
            return point.x >= min.x && point.x <= max.x &&
                    point.y >= min.y && point.y <= max.y;
        }

        [[nodiscard]] ivec2 center() const { return (min + max) / 2; }
        [[nodiscard]] uvec2 size() const { return max - min; }



        [[nodiscard]] aabb operator+(const ivec2 rhs) const { return { min + rhs, max + rhs }; }
        [[nodiscard]] aabb operator-(const ivec2 rhs) const { return { min - rhs, max - rhs }; }

        [[nodiscard]] aabb operator*(const ivec2 rhs) const { return { min * rhs, max * rhs }; }
        [[nodiscard]] aabb operator*(const vec2 rhs) const { return { vec2(min) * rhs, vec2(max) * rhs }; }

        [[nodiscard]] aabb operator/(const ivec2 rhs) const { return { min / rhs, max / rhs }; }
        [[nodiscard]] aabb operator/(const vec2 rhs) const { return { vec2(min) / rhs, vec2(max) / rhs }; }

        aabb& operator+=(const ivec2 rhs)
        {
            min += rhs;
            max += rhs;
            return *this;
        }

        aabb& operator-=(const ivec2 rhs)
        {
            min -= rhs;
            max -= rhs;
            return *this;
        }

        aabb& operator*=(const ivec2 rhs)
        {
            min *= rhs;
            max *= rhs;
            return *this;
        }

        aabb& operator/=(const ivec2 rhs)
        {
            min /= rhs;
            max /= rhs;
            return *this;
        }
    };

    inline aabb aabb::screen{ {}, Target::RESOLUTION };
}
