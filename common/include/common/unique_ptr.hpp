#pragma once

#include <cassert>
#include <utility>

namespace mse
{
    /**
     * @brief A smart pointer that owns and manages another object through a raw pointer
     *
     * Example:
     * @code
     * unique_ptr<int> ptr = make_unique<int>(5);
     * unique_ptr<int> ptr2 = std::move(ptr);
     * unique_ptr<int> ptr3{ 10 };
     * unique_ptr<int> ptr4{ new int{ 10 } };
     * @endcode
     */
    template <typename T>
    struct unique_ptr final
    {
        unique_ptr() = default;
        unique_ptr(std::nullptr_t) {}

        ~unique_ptr() { delete ptr_; }

        /// @brief Constructs a unique_ptr that owns the given pointer
        unique_ptr(T* ptr) : ptr_{ ptr } {}

        /// @brief Constructs a unique_ptr that owns a new object of type T constructed with the given arguments
        template <typename... Args>
        explicit unique_ptr(Args&&... args) : ptr_{ new T(std::forward<Args>(args)...) } {}

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

        [[nodiscard]]
        T& operator*()
        {
            assert(ptr_ && "dereferencing nullptr");
            return *ptr_;
        }

        [[nodiscard]]
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

    /**
     * @brief Constructs a unique_ptr that owns a new object of type T constructed with the given arguments
     * @tparam T The type of the object to be constructed
     * @tparam Args The types of the arguments to be passed to the constructor of T
     * @param args The arguments to be passed to the constructor of T
     * @return unique_ptr<T> A unique_ptr that owns a new object of type T constructed with the given arguments
     */
    template <typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) { return unique_ptr<T>{ new T(std::forward<Args>(args)...) }; }
}
