#pragma once

#include <concepts>
#include <utility>
#include <cassert>
#include <cstddef>

namespace mse
{
    /**
     * @brief A dynamic array implementation
     * @tparam T The type of the elements stored in the vector
     */
    template <typename T = void>
    class vector;

    template <>
    class vector<void>
    {
    public:
        static constexpr size_t npos = static_cast<size_t>(-1);
    };

    template <typename T> requires (!std::same_as<T, void>)
    class vector<T>
    {
    public:
        vector() = default;
        vector(std::nullptr_t) {}

        /**
         * @brief Constructs a vector with the specified number of elements
         * @tparam C The type of the size parameter
         * @param size The number of elements to construct
         */
        template <std::integral C>
        explicit vector(C size);

        /**
         * @brief Constructs a vector with the specified elements
         * @tparam Args The types of the arguments to be passed to the constructor of T
         * @param args Elements convertable to T
         * @note If T is integral and only one argument is provided, the argument is treated as the size of the vector
         */
        template <typename... Args> requires (
            (std::convertible_to<Args, T> && ...) &&
            !(sizeof...(Args) == 1 && (std::integral<Args> && ...)))
        vector(Args&&... args);

        ~vector();

        vector(const vector& other);
        vector(vector&& other) noexcept;
        vector& operator=(const vector& other);
        vector& operator=(vector&& other) noexcept;

        /// @brief Accesses the element at the specified index
        /// @note No bounds checking is performed
        T& operator[](size_t idx) { return data_[idx]; }

        /// @brief Accesses the element at the specified index
        /// @note No bounds checking is performed
        const T& operator[](size_t idx) const { return data_[idx]; }

        T& front() { assert(capacity_ && data_); return data_[0]; }
        const T& front() const { assert(capacity_ && data_); return data_[0]; }

        T& back() { assert(size_ && data_); return data_[size_ - 1]; }
        const T& back() const { assert(size_ && data_); return data_[size_ - 1]; }

        /// @brief Resets the vector to an empty state (does deallocate memory)
        void reset();

        /// @brief Resets the vector to an empty state (does not deallocate memory)
        void clear();

        bool empty() const { return !size_; }

        T* data() { return data_; }
        const T* data() const { return data_; }
        size_t size() const { return size_; }
        size_t capacity() const { return capacity_; }

        /// @brief Inserts a copy of the specified value to the front of the vector
        void push_front(const T& value) { emplace_front(value); }

        /// @brief Inserts a copy of the specified value at the specified index
        void push(size_t idx, const T& value) { emplace(idx, value); }

        /// @brief Inserts a copy of the specified value at the back of the vector
        void push_back(const T& value) { emplace_back(value); }


        /// @brief Moves the specified value to the front of the vector
        void push_front(T&& value) { emplace_front(std::move(value)); }

        /// @brief Moves the specified value to the specified index of the vector
        void push(size_t idx, T&& value) { emplace(idx, std::move(value)); }

        /// @brief Moves the specified value to the back of the vector
        void push_back(T&& value) { emplace_back(std::move(value)); }


        /**
         * @brief Constructs a new element in-place at the front of the vector
         * @tparam Args Arguments to construct the new element
         * @param args Arguments to construct the new element
         */
        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace_front(Args&&... args);

        /**
         * @brief Constructs a new element in-place at the specified index of the vector
         * @tparam Args Arguments to construct the new element
         * @param idx The index at which to construct the new element
         * @param args Arguments to construct the new element
         */
        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace(size_t idx, Args&&... args);

        /**
        * @brief Constructs a new element in-place at the back of the vector
        * @tparam Args Arguments to construct the new element
        * @param args Arguments to construct the new element
        */
        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace_back(Args&&... args);

        void pop_front();
        void pop(size_t idx);
        void pop_back();

        /**
         * @brief Reallocates the vector's buffer to the specified capacity
         * @param new_cap New capacity
         * @note The method always makes a new allocation for the vector's data
         * Make sure you don't keep a pointer to the vector's data after calling this method
         */
        void reserve(size_t new_cap);

        /**
        * @brief Resizes the vector to the specified size
        * @details
        * Doubles the vector's capacity until the new size is smaller than it
        * If the new size is smaller than the current capacity / 4, the vector's buffer is downsized in half
        * @param new_size New size
        * @param fill Value to fill the new elements with
        * @note The method may make a reallocation for the vector's data,
        * don't keep a pointer to the vector's data after calling this method
        */
        void resize(size_t new_size, T fill = {});

        /**
         * @brief Finds the index of the first occurrence of a value in the vector
         * @param value The value to search for
         * @return The index of the first occurrence of the value, or vector<>::npos if not found
         */
        size_t find(const T& value) requires std::equality_comparable<T>
        {
            for (size_t i = 0; i < size_; ++i)
            {
                if (data_[i] == value) return i;
            }

            return vector<>::npos;
        }

        // iterator methods
        T* begin() { return data_; }
        T* end() { return data_ + size_; }

        const T* begin() const { return data_; }
        const T* end() const { return data_ + size_; }

        const T* cbegin() const { return data_; }
        const T* cend() const { return data_ + size_; }

    private:
        // helpers for copying, moving and reallocation
        static void copy_mem(T* dest, const T* src, size_t count);
        static void move_mem(T* dest, T* src, size_t count);
        static void move_realloc(T* dest, T* src, size_t count);

        size_t size_{ 0 };
        size_t capacity_{ 0 };
        T* data_{ nullptr };
    };
}

#include "vector.inl"
