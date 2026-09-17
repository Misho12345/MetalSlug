#pragma once
#include <type_traits>

namespace mse
{
    // I'm not sure if it was allowed to use std::destroy_at or not, so I implemented one myself
    // strong inspiration from clang's implementation (copied)
    template <typename T>
    constexpr void destroy_at(T* ptr)
    {
        if constexpr (std::is_array_v<T>)
        {
            for (T& elem : *ptr) destroy_at(&elem);
        }
        else ptr->~T();
    }
}