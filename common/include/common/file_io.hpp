#pragma once

#include <cstdint>

#include "api.hpp"
#include "span.hpp"
#include "string.hpp"
#include "string_view.hpp"

namespace mse
{
    class MSC_API FileIO final
    {
    public:
        [[nodiscard]]
        static string read(string_view path);
        static void   write(string_view path, string_view data);
        static void   write(string_view path, span<const uint8_t> data);
    };
}
