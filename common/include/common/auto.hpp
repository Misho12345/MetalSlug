#pragma once
#include <utility>

namespace mse
{
    // for lambdas and stuff
    // not for pointers, references and other non pure class types
    template <typename T> requires std::is_class_v<T> && (!std::is_final_v<T>)
    struct Auto : T
    {
        Auto(T&& v) : T{ std::move(v) } {}
    };
}
