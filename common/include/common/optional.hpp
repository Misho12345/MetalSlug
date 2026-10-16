#pragma once
#include "memory.hpp"

#include <new>
#include <utility>
#include <cstdint>

namespace mse
{
    namespace detail
    {
        template <typename T> concept cpy_ctr = std::copy_constructible<T>;
        template <typename T> concept mv_ctr = std::move_constructible<T>;
        template <typename T> concept triv_cpy = std::is_trivially_copyable_v<T>;
        template <typename T> concept nothr_mv = std::is_nothrow_move_constructible_v<T>;
    }

    struct nullopt_t{};

    constexpr inline nullopt_t nullopt{};

    template <typename T>
    struct optional final
    {
        optional() = default;
        optional(nullopt_t) {}

        ~optional() requires std::is_trivially_destructible_v<T> = default;
        ~optional() { destroy(); }

        optional(const optional&)            requires detail::cpy_ctr<T> && detail::triv_cpy<T> = default;
        optional(optional&&)                 requires detail::mv_ctr<T>  && detail::triv_cpy<T> = default;
        optional& operator=(const optional&) requires detail::cpy_ctr<T> && detail::triv_cpy<T> = default;
        optional& operator=(optional&&)      requires detail::mv_ctr<T>  && detail::triv_cpy<T> = default;



        optional(const optional& other) requires detail::cpy_ctr<T> { if (other.has_value) construct(*other); }
        optional(optional&& other) noexcept(detail::nothr_mv<T>) requires detail::mv_ctr<T>
        {
            if (other.has_value) construct(std::move(*other));
        }


        optional& operator=(const optional& other) requires detail::cpy_ctr<T>
        {
            if (&other == this) return *this;

            destroy();
            if (other.has_value) construct(*other);

            return *this;
        }

        optional& operator=(optional&& other) noexcept(detail::nothr_mv<T>) requires detail::mv_ctr<T>
        {
            if (&other == this) return *this;

            destroy();
            if (other.has_value) construct(std::move(*other));

            return *this;
        }


        optional(const T& other) requires detail::cpy_ctr<T> { construct(other); }
        optional(T&& other) noexcept(detail::nothr_mv<T>) requires detail::mv_ctr<T> { construct(std::move(other)); }


        optional& operator=(const T& other) requires detail::cpy_ctr<T>
        {
            if (static_cast<const void*>(&other) == data) return *this;

            destroy();
            construct(other);

            return *this;
        }

        optional& operator=(T&& other) noexcept(detail::nothr_mv<T>) requires detail::mv_ctr<T>
        {
            if (static_cast<const void*>(&other) == data) return *this;

            destroy();
            construct(std::move(other));

            return *this;
        }



        template <typename... Args> requires
            std::constructible_from<T, Args...> &&
            (!std::same_as<std::remove_cvref_t<Args>, optional> && ...)
        optional(Args&&... args) { construct(std::forward<Args>(args)...); }



        void destroy() noexcept
        {
            if (!has_value) return;
            mse::destroy_at(ptr());
            has_value = false;
        }

        explicit operator bool() const { return has_value; }

        T& operator*() { assert(has_value); return *ptr(); }
        const T& operator*() const { assert(has_value); return *ptr(); }

        T* operator->() { assert(has_value); return ptr(); }
        const T* operator->() const { assert(has_value); return ptr(); }


        alignas(T) uint8_t data[sizeof(T)];
        bool has_value{ false };

    private:
        T*       ptr() { return std::launder(reinterpret_cast<T*>(data)); }
        const T* ptr() const { return std::launder(reinterpret_cast<const T*>(data)); }

        template <typename... Args>
        void construct(Args&&... args)
        {
            new(data) T(std::forward<Args>(args)...);
            has_value = true;
        }
    };
}