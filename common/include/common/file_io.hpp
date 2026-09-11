#pragma once
#include "api.hpp"
#include "string.hpp"
#include "string_view.hpp"

namespace mse
{
    class MSC_API FileIO final
    {
    public:
        static string read(string_view path);
        static void write(string_view path, string_view data);
    };
}
