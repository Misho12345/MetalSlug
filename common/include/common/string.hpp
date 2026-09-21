#pragma once
#include "api.hpp"
#include <cstddef>

namespace mse
{
    /// @brief Defines a dynamic character array
    class MSC_API string final
    {
    public:
        /**
         * @brief Constructs a string from a null-terminated character array
         * @param data The null-terminated character array to initialize the string with
         */
        string(const char* data = nullptr);
        ~string();

        /**
         * @brief Constructs a string with a specified initial capacity
         * @param init_cap The initial capacity of the string
         * @note If you want to construct a string with an initial size,
         * use the constructor that takes a size and a fill character.
         */
        explicit string(size_t init_cap);

        /**
         * @brief Constructs a string with a specified initial size and fill character
         * @param size The initial size of the string
         * @param fill The character to fill the string with
         */
        string(size_t size, char fill);

        string(const string& other);
        string(string&& other) noexcept;
        string& operator=(const string& other);
        string& operator=(string&& other) noexcept;

        /**
         * @brief Accesses the character at the specified index
         * @param idx The index of the character to access
         * @return A reference to the character at the specified index
         * @note This operator does not perform bounds checking.
         */
        [[nodiscard]]
        char& operator[](const size_t idx) { return data_[idx]; }

        /**
         * @brief Accesses the character at the specified index
         * @param idx The index of the character to access
         * @return A const reference to the character at the specified index
         * @note This operator does not perform bounds checking.
         */
        [[nodiscard]]
        const char& operator[](const size_t idx) const { return data_[idx]; }

        /**
         * @brief Resets the string to an empty state
         * @note This function does deallocate memory.
         */
        void reset();


        [[nodiscard]]
        bool empty() const { return size_ == 0 || data_ == nullptr || data_[0] == '\0'; }

        [[nodiscard]] char* data() { return data_; }
        [[nodiscard]] const char* data() const { return data_; }
        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] size_t capacity() const { return capacity_; }

        [[nodiscard]] char front() const { return data_[0]; }
        [[nodiscard]] char back() const { return data_[size_ - 1]; }

        void push_front(char value);
        void push(char value, size_t idx);
        void push_back(char value);

        void pop_front();
        void pop(size_t idx);
        void pop_back();

        /**
         * @brief Reallocates the string's buffer to the specified capacity
         * @param new_cap New capacity
         * @note The method always makes a new allocation for the string's data
         * Make sure you don't keep a pointer to the string's data after calling this method
         */
        void reserve(size_t new_cap);

        /**
         * @brief Resizes the string to the specified size
         * @details
         * Doubles the string's capacity until the new size is smaller than it
         * If the new size is smaller than the current capacity / 4, the string's buffer is downsized in half
         * @param new_size New size
         * @note The method may make a reallocation for the string's data,
         * don't keep a pointer to the string's data after calling this method
         */
        void resize(size_t new_size);

        [[nodiscard]] string operator+(const string& other) const;
        [[nodiscard]] string operator+(const char* other) const;

        string& operator+=(const string& other);
        string& operator+=(const char* other);

        operator bool() const { return data_; }

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


