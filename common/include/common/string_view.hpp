#pragma once
#include <cstring>

#include "string.hpp"

namespace mse
{
    class MSC_API string_view final
    {
    public:
        template <size_t N>
        string_view(const char (&data)[N]) : data_{ data }, size_{ N } {}
        string_view(const char* data, const size_t size) : data_{ data }, size_{ size } {}
        string_view(const char* data) : data_{ data }, size_{ strlen(data) } {}
        string_view(const string& str) : data_{ str.data() }, size_{ str.size() } {}

        ~string_view() = default;

        operator bool() const { return data_; }

        const char& operator[](const size_t idx) const { return data_[idx]; }

        const char* data() const { return data_; }

        size_t size() const { return size_; }
        bool   empty() const { return size_ == 0; }

        char front() const { return data_[0]; }
        char back() const { return data_[size_ - 1]; }

        operator const char*() const { return data_; }

    private:
        const char* data_;
        size_t      size_;
    };

    namespace literals
    {
        inline string_view operator""_sv(const char* str, const size_t size) { return { str, size }; }
    }
}
