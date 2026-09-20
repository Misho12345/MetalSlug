#pragma once

#include <cstring>
#include "string.hpp"

namespace mse
{
    /**
     * @brief A non-owning view over a string
     *
     * Example:
     * @code
     * string str = "12345789";
     * string_view sv = "Hello";
     * string_view sv2 = str;
     * string_view sv3 = { str.data() + 3, 4 };
     * @endcode
     */
    class MSC_API string_view final
    {
    public:
        string_view() = default;

        string_view(const string_view&) = default;
        string_view& operator=(const string_view&) = default;

        /// @brief Construct a string view from a string literal
        template <size_t N>
        string_view(const char (&data)[N]) : data_{ data }, size_{ N } {}

        /// @brief Construct a string view from a pointer and a size
        string_view(const char* data, const size_t size) : data_{ data }, size_{ size } {}

        /// @brief Construct a string view from a pointer
        /// @note Make sure the pointer is valid and points to a null-terminated string
        string_view(const char* data) : data_{ data }, size_{ strlen(data) } {}

        /// @brief Construct a string view from a string
        string_view(const string& str) : data_{ str.data() }, size_{ str.size() } {}

        ~string_view() = default;

        operator bool() const { return data_; }

        /// @brief Access an element in the string view
        /// @note No out-of-bounds checks are made, make sure the index is within bounds
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
