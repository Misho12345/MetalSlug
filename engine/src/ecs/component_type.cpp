#include "mse/pch.hpp"
#include "mse/ecs/component_type.hpp"

namespace mse::detail
{
    MSE_API size_t register_component_type(const std::type_info& type)
    {
        static vector<const std::type_info*> types;

        // compare by value not address because typeid() can give different addresses across dll boundaries
        if (const size_t pos = types.find([&](const std::type_info* v) { return type == *v; });
            pos != vector<>::npos)
            return pos;

        types.emplace_back(&type);
        return types.size() - 1;
    }
}
