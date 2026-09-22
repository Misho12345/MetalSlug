#include "common/file_io.hpp"

#include <cstdio>

#include "common/span.hpp"

namespace mse
{
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
