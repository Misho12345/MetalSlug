#include "common/string.hpp"

#include <cstddef>
#include <cstring>
#include <utility>

#include "common/algorithm.hpp"

namespace mse
{
    string::string(const char* data) :
        size_{ data ? strlen(data) : 0 },
        capacity_{ data ? size_ + 1 : 0 },
        data_{ data ? new char[capacity_] : nullptr }
    {
        if (data_) memcpy(data_, data, capacity_ * sizeof(char));
    }

    string::~string() { delete[] data_; }

    string::string(const size_t init_cap) :
        capacity_{ max(init_cap, static_cast<size_t>(1)) },
        data_{ new char[capacity_] } { data_[0] = '\0'; }

    string::string(const size_t size, const char fill) : string{ size + 1 }
    {
        memset(data_, fill, capacity_ * sizeof(char));
    }

    string::string(const string& other)
    {
        if (other.empty()) return;

        size_     = other.size_;
        capacity_ = other.capacity_;
        data_     = new char[capacity_];
        memcpy(data_, other.data_, capacity_ * sizeof(char));
    }

    string::string(string&& other) noexcept :
        size_{ std::exchange(other.size_, 0) },
        capacity_{ std::exchange(other.capacity_, 0) },
        data_{ std::exchange(other.data_, nullptr) } {}

    string& string::operator=(const string& other)
    {
        if (this == &other) return *this;

        if (other.empty())
        {
            reset();
            return *this;
        }

        delete[] data_;

        size_     = other.size_;
        capacity_ = other.capacity_;
        data_     = new char[capacity_];

        memcpy(data_, other.data_, capacity_ * sizeof(char));

        return *this;
    }

    string& string::operator=(string&& other) noexcept
    {
        if (this == &other) return *this;

        delete[] data_;
        size_     = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        data_     = std::exchange(other.data_, nullptr);

        return *this;
    }

    void string::reset()
    {
        delete[] data_;

        size_     = 0;
        capacity_ = 0;
        data_     = nullptr;
    }


    void string::push_front(const char value) { push(value, 0); }

    void string::push(const char value, const size_t idx)
    {
        if (idx == size_) push_back(value);
        else
        {
            resize(size_ + 1);
            memmove(data_ + idx + 1, data_ + idx, sizeof(char) * (size_ - idx));
            data_[idx] = value;
        }
    }

    void string::push_back(const char value)
    {
        resize(size_ + 1);
        data_[size_ - 1] = value;
    }


    void string::pop_front() { pop(0); }

    void string::pop(const size_t idx)
    {
        if (idx == size_ - 1) pop_back();
        else if (size_)
        {
            memmove(data_ + idx, data_ + idx + 1, sizeof(char) * (size_ - idx - 1));
            data_[--size_] = '\0';
        }
    }

    void string::pop_back() { if (size_) resize(size_ - 1); }


    void string::reserve(size_t new_cap)
    {
        if (new_cap == 0) new_cap = 1;

        if (size_ >= new_cap)
        {
            size_ = new_cap - 1;
            data_[size_] = '\0';
        }

        char* new_data = new char[new_cap];

        if (data_)
        {
            memcpy(new_data, data_, sizeof(char) * (size_ + 1));
            delete[] data_;
        }

        data_     = new_data;
        capacity_ = new_cap;
    }

    void string::resize(const size_t new_size)
    {
        if (new_size <= size_)
        {
            size_ = new_size;
            data_[size_] = '\0';
        }
        else if (new_size >= capacity_)
        {
            size_t new_cap = min(capacity_, static_cast<size_t>(1));
            while (new_size >= new_cap) new_cap *= 2;
            reserve(new_cap);
        }
        else if (new_size + 1 < capacity_ / 2)
        {
            reserve(capacity_ / 2);
        }

        size_ = new_size;
        data_[size_] = '\0';
    }

    string string::operator+(const string& other) const
    {
        string new_str{ *this };
        new_str += other;
        return new_str;
    }

    string string::operator+(const char* other) const
    {
        string new_str{ *this };
        new_str += other;
        return new_str;
    }

    string& string::operator+=(const string& other)
    {
        if (other.empty()) return *this;

        const size_t other_size = other.size();
        const size_t tmp = size_;
        const bool same = data_ == other.data_;

        resize(size_ + other_size);

        memcpy(data_ + tmp, same ? data_ : other.data(), sizeof(char) * other_size);
        data_[size_] = '\0';

        return *this;
    }

    string& string::operator+=(const char* other)
    {
        if (!other || !other[0]) return *this;

        const size_t other_size = strlen(other);
        const size_t tmp = size_;

        const bool overlap = data_ >= other && data_ < other + other_size;
        const ptrdiff_t offset = overlap ? data_ - other : 0;

        resize(size_ + other_size);
        memcpy(data_ + tmp, overlap ? data_ + offset : other, sizeof(char) * other_size);
        data_[size_] = '\0';

        return *this;
    }
}
