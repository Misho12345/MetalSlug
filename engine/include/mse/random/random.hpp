#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct GameCtx;

    /**
     * @brief libc random wrapper
     * @note Owned by GameCtx, not constructable by anything else and not movable or copyable
     */
    class Random final
    {
    public:
        Random(const Random&)            = delete;
        Random(Random&&)                 = delete;
        Random& operator=(const Random&) = delete;
        Random& operator=(Random&&)      = delete;

        template<std::integral T = int32_t>
        [[nodiscard]]
        T range(const T min, const T max) const { return min + (T)rand() % (max - min + 1); }

        template<std::floating_point T = float>
        [[nodiscard]]
        T range(const T min, const T max) const { return min + (max - min) * real<T>(); }

        template<std::floating_point T = float>
        [[nodiscard]]
        T real() const { return (T)rand() / ((T)RAND_MAX + (T)1); }

    private:
        Random()
        {
            #ifndef NDEBUG // predictable on Debug
            srand(67);
            #else
            srand((uint32_t)time(NULL));
            #endif
        }

        friend ::mse::GameCtx;
    };
}