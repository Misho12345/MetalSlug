#include "common/file_io.hpp"

#include <cstdio>

namespace mse
{
    string FileIO::read(const string_view path)
    {
        if (path.empty()) return {};

        string out;
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

    void FileIO::write(const string_view path, const string_view data)
    {
        FILE* f = fopen(path.data(), "wb");
        fwrite(data.data(), sizeof(char), data.size(), f);
        fclose(f);
    }
}
