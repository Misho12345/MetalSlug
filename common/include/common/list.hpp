#pragma once
#include <concepts>
#include <utility>

#include <cassert>
#include <cstddef>

namespace mse
{
    template <typename T>
    class list final
    {
    public:
        struct node final
        {
            template <typename... Args> requires std::constructible_from<T, Args...>
            node(node* prev, node* next, Args&&... args)
                : data{ std::forward<Args>(args)... },
                  prev{ prev },
                  next{ next } {}

            T data;
            node* prev{ nullptr };
            node* next{ nullptr };
        };

        template <typename... Args> requires (std::convertible_to<Args, T> && ...)
        list(Args&&... args);

        ~list();

        list(const list& other);
        list(list&& other) noexcept;
        list& operator=(const list& other);
        list& operator=(list&& other) noexcept;


        T& operator[](size_t idx);
        const T& operator[](size_t idx) const;


        T& front() { assert(head_); return head_->data; }
        const T& front() const { assert(head_); return head_->data; }

        T& back() { assert(tail_); return tail_->data; }
        const T& back() const { assert(tail_); return tail_->data; }



        node* head() { return head_; }
        const node* head() const { return head_; }

        node* tail() { return tail_; }
        const node* tail() const { return tail_; }


        node* get_node(const size_t idx)
        {
            return const_cast<node*>(std::as_const(this)->get_node(idx));
        }

        const node* get_node(size_t idx) const;


        void reset();
        bool empty() const { return !size_; }
        size_t size() const { return size_; }


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

        static void swap_elements(node* n1, node* n2);

    private:
        node* head_{ nullptr };
        node* tail_{ nullptr };
        size_t size_{ 0 };
    };
}

#include "list.inl"