#pragma once
#include <cstring>
#include <new>

#include "algorithm.hpp"
#include "memory.hpp"

namespace mse
{
    template <typename T>
    template <std::integral C>
    vector<T>::vector(const C size) :
        size_{ static_cast<size_t>(size) },
        capacity_{ static_cast<size_t>(size) },
        data_{ static_cast<T*>(operator new(sizeof(T) * capacity_)) }
    {
        for (size_t i = 0; i < size_; ++i) new(data_ + i) T{};
    }

    template <typename T>
    template <typename... Args> requires (
        (std::convertible_to<Args, T> && ...) &&
        !(sizeof...(Args) == 1 && (std::integral<Args> && ...)))
    vector<T>::vector(Args&&... args) :
        size_{ sizeof...(Args) },
        capacity_{ sizeof...(Args) },
        data_{ static_cast<T*>(operator new(sizeof(T) * capacity_)) }
    {
        size_t idx{};
        (new(data_ + idx++) T{ std::forward<Args>(args) }, ...);
    }

    template <typename T>
    vector<T>::~vector() { reset(); }

    template <typename T>
    vector<T>::vector(const vector& other) : size_{ other.size_ }, capacity_{ other.capacity_ }
    {
        data_ = static_cast<T*>(::operator new(sizeof(T) * other.capacity_));
        copy_mem(data_, other.data_, other.size_);
    }

    template <typename T>
    vector<T>::vector(vector&& other) noexcept :
        size_{ std::exchange(other.size_, 0) },
        capacity_{ std::exchange(other.capacity_, 0) },
        data_{ std::exchange(other.data_, nullptr) } {}

    template <typename T>
    vector<T>& vector<T>::operator=(const vector& other)
    {
        if (this == &other) return *this;

        T* new_data = static_cast<T*>(::operator new(sizeof(T) * other.capacity_));
        copy_mem(data_, other.data_, other.size_);

        reset();
        data_ = new_data;
        size_ = other.size_;
        capacity_ = other.capacity_;

        return *this;
    }

    template <typename T>
    vector<T>& vector<T>::operator=(vector&& other) noexcept
    {
        if (this == &other) return *this;

        reset();
        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);

        return *this;
    }

    template <typename T>
    void vector<T>::reset()
    {
        for (size_t i = 0; i < size_; ++i) destroy_at(data_ + i);
        ::operator delete(data_);

        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }


    template <typename T>
    template <typename... Args> requires std::constructible_from<T, Args...>
    void vector<T>::emplace_front(Args&&... args)
    {
        emplace(std::forward<Args>(args)..., 0);
    }

    template <typename T>
    template <typename... Args> requires std::constructible_from<T, Args...>
    void vector<T>::emplace(size_t idx, Args&&... args)
    {
        if (idx == size_) emplace_back(std::forward<Args>(args)...);
        else
        {
            if (capacity_ < size_ + 1) reserve(capacity_ * 2);

            new(data_ + size_) T{ std::move(data_[size_ - 1]) };
            move_mem(data_ + idx + 1, data_ + idx, size_ - idx - 1);
            destroy_at(data_ + idx);
            new(data_ + idx) T{ std::forward<Args>(args)... };
            ++size_;
        }
    }

    template <typename T>
    template <typename... Args> requires std::constructible_from<T, Args...>
    void vector<T>::emplace_back(Args&&... args)
    {
        if (capacity_ < size_ + 1) reserve(capacity_ * 2);
        new(data_ + size_++) T{ std::forward<Args>(args)... };
    }


    template <typename T>
    void vector<T>::pop_front() { pop(0); }

    template <typename T>
    void vector<T>::pop(size_t idx)
    {
        if (idx == size_ - 1) pop_back();
        else if (size_)
        {
            move_mem(data_ + idx, data_ + idx + 1, size_ - idx - 1);
            destroy_at(data_ + --size_);
        }
    }

    template <typename T>
    void vector<T>::pop_back()
    {
        destroy_at(data_ + --size_);
    }


    template <typename T>
    void vector<T>::reserve(size_t new_cap)
    {
        if (new_cap == 0) new_cap = 1;

        if (size_ > new_cap)
        {
            for (size_t i = new_cap; i < size_; ++i)
                destroy_at(data_ + i);
            size_ = new_cap;
        }

        T* new_data = static_cast<T*>(operator new(sizeof(T) * new_cap));
        move_realloc(new_data, data_, size_);
        ::operator delete(data_);

        data_ = new_data;
        capacity_ = new_cap;
    }

    template <typename T>
    void vector<T>::resize(const size_t new_size)
    {
        if (new_size <= size_)
        {
            for (size_t i = new_size; i < size_; ++i) destroy_at(data_ + i);
            size_ = new_size;
            if (size_ < capacity_ / 4) reserve(capacity_ / 2);
            return;
        }

        if (new_size > capacity_)
        {
            size_t new_cap = min(capacity_, static_cast<size_t>(1));
            while (new_size > new_cap) new_cap *= 2;
            reserve(new_cap);
        }

        for (size_t i = size_; i < new_size; ++i) new(data_ + i) T{};
        size_ = new_size;
    }

    template <typename T>
    void vector<T>::copy_mem(T* dest, const T* src, const size_t count)
    {
        if (!dest || !src || dest == src) return;

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            memcpy(dest, src, sizeof(T) * count);
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                new(dest + i) T{ src[i] };
            }
        }
    }

    template <typename T>
    void vector<T>::move_mem(T* dest, T* src, const size_t count)
    {
        if (!dest || !src || dest == src) return;

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            memmove(dest, src, sizeof(T) * count);
        }
        else
        {
            if (dest < src)
            {
                for (size_t i = 0; i < count; ++i)
                {
                    dest[i] = std::move(src[i]);
                }
            }
            else if (dest > src)
            {
                for (size_t i = count; i > 0; --i)
                {
                    dest[i - 1] = std::move(src[i - 1]);
                }
            }
        }
    }

    template <typename T>
    void vector<T>::move_realloc(T* dest, T* src, const size_t count)
    {
        if (!dest || !src || dest == src) return;

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            std::memcpy(dest, src, sizeof(T) * count);
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                new(dest + i) T{ std::move(src[i]) };
                destroy_at(src + i);
            }
        }
    }
}
