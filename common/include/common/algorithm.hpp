#pragma once

namespace mse
{
    template <typename T> [[nodiscard]] constexpr T min(const T& v) { return v; }
    template <typename T> [[nodiscard]] constexpr T max(const T& v) { return v; }

    template <typename T, std::convertible_to<T>... Args> requires (sizeof...(Args) >= 1)
    [[nodiscard]]
    constexpr T min(T head, Args&&... other)
    {
        const T v = min(other...);
        return head < v ? head : v;
    }

    template <typename T, std::convertible_to<T>... Args> requires (sizeof...(Args) >= 1)
    [[nodiscard]]
    constexpr T max(T head, Args&&... other)
    {
        const T v = max(other...);
        return head > v ? head : v;
    }

    template <typename T>
    [[nodiscard]]
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