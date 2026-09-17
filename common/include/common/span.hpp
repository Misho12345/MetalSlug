#pragma once
#include "vector.hpp"

namespace mse
{
    /**
     * @brief A non-owning view over a contiguous sequence of elements
     * @tparam T The type of the elements in the sequence
     *
     * Example:
     * @code
     * T arr[10];
     * vector<T> vec;
     *
     * span<T> s1{ vec };
     * span<T> s2{ arr };
     * span<T> s3{ arr + 2, 5 };
     * @endcode
     */
    template <typename T>
    class span final
    {
    public:
        /// @brief Construct a span from a pointer and a size
        span(T* data, const size_t size) : data_{ data }, size_{ size } {}

        /// @brief Construct a span from a vector
        /// @details This is needed to allow constructing a span<T> from a vector<T>
        span(vector<T>& vec) : data_{ vec.data() }, size_{ vec.size() } {}

        /// @brief Construct a span from a const vector
        /// @details This is needed to allow constructing a span<const T> from a const vector<T>
        span(const vector<std::remove_const_t<T>>& vec) : data_{ vec.data() }, size_{ vec.size() } {}

        /// @brief Construct a span from a C-style array
        template <size_t N>
        span(T (&str)[N]) : data_{ str }, size_{ N } {}

        ~span() = default;

        /// @brief Check if the span is valid
        operator bool() const { return data_; }

        /**
         * @brief Access an element in the span
         * @param idx The index of the element to access
         * @return A reference to the element at the specified index
         * @note This function does not perform bounds checking
         */
        T& operator[](const size_t idx) const { return data_[idx]; }

        T* data() const { return data_; }
        size_t size() const { return size_; }
        bool   empty() const { return size_ == 0; }


        // iterator methods
        T& front() const { return data_[0]; }
        T& back() const { return data_[size_ - 1]; }

        T* begin() const { return data_; }
        T* end() const { return data_ + size_; }

    private:
        T*     data_;
        size_t size_;
    };
}
