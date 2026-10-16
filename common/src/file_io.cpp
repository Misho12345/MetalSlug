#include "common/file_io.hpp"
#include "common/span.hpp"

#include <cstdio>
#include <cstdlib>

namespace mse
{
    template <>
    FileIO::read_ret_t<Csv> FileIO::read<Csv>(const string_view path)
    {
        string data = read(path);
        if (data.empty()) return {};

        Csv out;
        uint32_t width = 0, height = 0;

        for (const char* c = data.data(); *c; ++c)
        {
            if (*c == '\r') continue;
            if (*c == '\n') ++height;
            else if (!height && *c == ',') ++width;
        }

        ++width; // n commas -> n + 1 numbers

        out.width = width;
        out.height = height;
        out.data.reserve(width * height);


        for (const char* c = data.data(); *c; )
        {
            out.data.emplace_back(atoi(c));
            
            c = strpbrk(c, ",\n\r");
            if (!c) break;

            while (*c == ',' || *c == '\n' || *c == '\r') ++c;
        }

        return out;
    }


    void FileIO::write(const string_view path, const string_view data)
    {
        write(path, { reinterpret_cast<const uint8_t*>(data.data()), data.size() });
    }

    void FileIO::write(const string_view path, const span<const uint8_t> data)
    {
        FILE* f = fopen(path.data(), "wb");
        fwrite(data.data(), sizeof(char), data.size(), f);
        fclose(f);
    }
}
