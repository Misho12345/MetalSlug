#pragma once
#include <cassert>
#include <utility>

namespace mse
{
    template <typename T>
    struct unique_ptr final
    {
        unique_ptr() = default;
        ~unique_ptr() { delete ptr_; }

        unique_ptr(T* ptr) : ptr_{ ptr } {}

        template <typename... Args>
        explicit unique_ptr(Args&&... args) : ptr_{ new T{ std::forward<Args>(args)... } } {}

        unique_ptr(const unique_ptr&)            = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;

        unique_ptr(unique_ptr&& other) noexcept : ptr_{ std::exchange(other.ptr_, nullptr) } {}

        unique_ptr& operator=(unique_ptr&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                ptr_ = std::exchange(other.ptr_, nullptr);
            }

            return *this;
        }

        T& operator*()
        {
            assert(ptr_ && "dereferencing nullptr");
            return *ptr_;
        }

        const T& operator*() const
        {
            assert(ptr_ && "dereferencing nullptr");
            return *ptr_;
        }

        T* operator->()
        {
            assert(ptr_ && "dereferencing nullptr");
            return ptr_;
        }

        void reset()
        {
            delete ptr_;
            ptr_ = nullptr;
        }

    private:
        T* ptr_{ nullptr };
    };

    template <typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) { return unique_ptr<T>{ new T{ std::forward<Args>(args)...} }; }
}
