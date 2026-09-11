#pragma once
#include "api.hpp"
#include <cstddef>

namespace mse
{
    class MSC_API string final
    {
    public:
        string(const char* data = nullptr);
        ~string();

        explicit string(size_t init_cap);
        string(size_t size, char fill);

        string(const string& other);
        string(string&& other) noexcept;
        string& operator=(const string& other);
        string& operator=(string&& other) noexcept;

        char& operator[](const size_t idx) { return data_[idx]; }
        const char& operator[](const size_t idx) const { return data_[idx]; }

        void reset();
        bool empty() const { return size_ == 0 || data_ == nullptr || data_[0] == '\0'; }

        char* data() { return data_; }
        const char* data() const { return data_; }
        size_t size() const { return size_; }
        size_t capacity() const { return capacity_; }

        char front() const { return data_[0]; }
        char back() const { return data_[size_ - 1]; }

        void push_front(char value);
        void push(char value, size_t idx);
        void push_back(char value);

        void pop_front();
        void pop(size_t idx);
        void pop_back();

        void reserve(size_t new_cap);
        void resize(size_t new_size);

        string operator+(const string& other) const;
        string operator+(const char* other) const;

        string& operator+=(const string& other);
        string& operator+=(const char* other);

        operator bool() const { return data_; }
        operator char*() { return data_; }

    private:
        size_t size_{ 0 };
        size_t capacity_{ 0 };
        char* data_{ nullptr };
    };

    namespace literals
    {
        inline string operator""_s(const char* str, size_t) { return string{ str }; }
    }
}


