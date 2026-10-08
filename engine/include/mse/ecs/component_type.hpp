#pragma once
#include "mse/api.hpp"
#include "mse/pch.hpp"

namespace mse
{
    namespace detail
    {
        MSE_API size_t register_component_type(const std::type_info& type);
    }

    template <typename C>
    size_t component_type_index()
    {
        static const size_t index = detail::register_component_type(typeid(C));
        return index;
    }
}
