#pragma once
#include <type_traits>

namespace mse
{
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