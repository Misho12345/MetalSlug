#pragma once
#include <concepts>
#include <utility>

#include <cstddef>

namespace mse
{
    template <typename T>
    class vector final
    {
    public:
        vector() = default;
        vector(std::nullptr_t) {}

        template <std::integral C>
        explicit vector(C size);

        template <typename... Args> requires (
            (std::convertible_to<Args, T> && ...) &&
            !(sizeof...(Args) == 1 && (std::integral<Args> && ...)))
        vector(Args&&... args);

        ~vector();

        vector(const vector& other);
        vector(vector&& other) noexcept;
        vector& operator=(const vector& other);
        vector& operator=(vector&& other) noexcept;

        T& operator[](size_t idx) { return data_[idx]; }
        const T& operator[](size_t idx) const { return data_[idx]; }

        T& front() { return data_[0]; }
        const T& front() const { return data_[0]; }

        T& back() { return data_[size_ - 1]; }
        const T& back() const { return data_[size_ - 1]; }

        void reset();
        bool empty() const { return !size_; }

        T* data() { return data_; }
        const T* data() const { return data_; }
        size_t size() const { return size_; }
        size_t capacity() const { return capacity_; }

        void push_front(const T& value) { emplace_front(value); }
        void push(size_t idx, const T& value) { emplace(idx, value); }
        void push_back(const T& value) { emplace_back(value); }

        void push_front(T&& value) { emplace_front(std::move(value)); }
        void push(size_t idx, T&& value) { emplace(idx, std::move(value)); }
        void push_back(T&& value) { emplace_back(std::move(value)); }

        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace_front(Args&&... args);

        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace(size_t idx, Args&&... args);

        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace_back(Args&&... args);

        void pop_front();
        void pop(size_t idx);
        void pop_back();

        void reserve(size_t new_cap);
        void resize(size_t new_size);

    private:
        static void copy_mem(T* dest, const T* src, size_t count);
        static void move_mem(T* dest, T* src, size_t count);
        static void move_realloc(T* dest, T* src, size_t count);

        size_t size_{ 0 };
        size_t capacity_{ 0 };
        T* data_{ nullptr };
    };
}

#include "vector.inl"
