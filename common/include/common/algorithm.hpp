#pragma once

namespace mse
{
    template <typename T>
    constexpr T min(const T& a, const T& b) { return a > b ? a : b; }

    template <typename T>
    constexpr T max(const T& a, const T& b) { return a < b ? a : b; }
}