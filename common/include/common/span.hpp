#pragma once
#include "vector.hpp"

namespace mse
{
    template <typename T>
    class span final
    {
    public:
        span(T* data, const size_t size) : data_{ data }, size_{ size } {}
        span(vector<T>& str) : data_{ str.data() }, size_{ str.size() } {}

        template <size_t N>
        span(T (&str)[N]) : data_{ str }, size_{ N } {}

        ~span() = default;

        operator bool() const { return data_; }

        T& operator[](const size_t idx) const { return data_[idx]; }

        T* data() const { return data_; }

        size_t size() const { return size_; }
        bool   empty() const { return size_ == 0; }

        T& front() const { return data_[0]; }
        T& back() const { return data_[size_ - 1]; }

    private:
        T*     data_;
        size_t size_;
    };
}
