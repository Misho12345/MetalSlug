#pragma once

#include <cstdint>
#include <cstdio>

#include "api.hpp"
#include "span.hpp"
#include "string.hpp"
#include "string_view.hpp"

namespace mse
{
    struct Csv final
    {
        vector<int32_t> data{};
        uint32_t width{}, height{};
    };

    class MSC_API FileIO final
    {
        template <typename T>
        using read_ret_t =
            std::conditional_t<std::same_as<T, char>, string,
            std::conditional_t<std::same_as<T, Csv>, Csv, vector<T>>>;

    public:
        template <typename C = char>
        [[nodiscard]]
        static read_ret_t<C> read(string_view path);


        static void write(string_view path, string_view data);
        static void write(string_view path, span<const uint8_t> data);
    };

    template <typename C>
    FileIO::read_ret_t<C> FileIO::read(const string_view path)
    {
        if (path.empty()) return {};

        read_ret_t<C> out;
        FILE* f = fopen(path.data(), "rb");

        if (!f) return out;

        fseek(f, 0, SEEK_END);
        const long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        out.resize((size_t)size);
        fread(out.data(), sizeof(char), (size_t)size, f);
        fclose(f);

        return out;
    }

    template <>
    MSC_API FileIO::read_ret_t<Csv> FileIO::read<Csv>(string_view path);
}
