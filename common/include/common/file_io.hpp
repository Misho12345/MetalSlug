#pragma once

#include <cstdint>
#include <cstdio>

#include "api.hpp"
#include "span.hpp"
#include "string.hpp"
#include "string_view.hpp"

namespace mse
{
    class MSC_API FileIO final
    {
    public:
        template <typename C = string>
        [[nodiscard]]
        static C read(const string_view path)
        {
            if (path.empty()) return {};

            C out;
            FILE* f = fopen(path.data(), "rb");

            if (!f) return out;

            fseek(f, 0, SEEK_END);
            const size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);

            out.resize(size);
            fread(out.data(), sizeof(char), size, f);
            fclose(f);

            return out;
        }

        static void write(string_view path, string_view data);
        static void write(string_view path, span<const uint8_t> data);
    };
}
