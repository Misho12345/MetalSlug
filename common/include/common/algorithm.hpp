#pragma once

namespace mse
{
    template <typename T>
    constexpr T min(const T& a, const T& b) { return a < b ? a : b; }

    template <typename T>
    constexpr T max(const T& a, const T& b) { return a > b ? a : b; }

    // signature practically copied from cppreference
    template <typename T> requires (
        std::is_move_constructible_v<T> &&
        std::is_move_assignable_v<T>)
    constexpr void swap(T& a, T& b) noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_move_assignable_v<T>)
    {
        T tmp = std::move(a);
        a = std::move(b);
        b = std::move(tmp);
    }
}