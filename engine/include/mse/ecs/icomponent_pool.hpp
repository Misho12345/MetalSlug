#pragma once
#include "entity_id.hpp"

namespace mse
{
    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;
        virtual void remove(entity_id id) = 0;
    };
}
