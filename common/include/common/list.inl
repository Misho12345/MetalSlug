#pragma once
#include <utility>

namespace mse
{
    template <typename T>
    template <typename... Args> requires (std::convertible_to<Args, T> && ...)
    list<T>::list(Args&&... args)
    {
        (emplace_back(std::forward<Args>(args)), ...);
    }

    template <typename T>
    list<T>::~list() { reset(); }


    template <typename T>
    list<T>::list(const list& other)
    {
        for (node* n = other.head_; n; n = n->next) emplace_back(n->data);
    }

    template <typename T>
    list<T>::list(list&& other) noexcept
        : head_{ std::exchange(other.head_, nullptr) },
          tail_{ std::exchange(other.tail_, nullptr) },
          size_{ std::exchange(other.size_, 0) } {}

    template <typename T>
    list<T>& list<T>::operator=(const list& other)
    {
        if (this == &other) return *this;

        reset();
        for (node* n = other.head_; n; n = n->next) emplace_back(n->data);

        return *this;
    }

    template <typename T>
    list<T>& list<T>::operator=(list&& other) noexcept
    {
        if (this == &other) return *this;
        reset();

        head_ = std::exchange(other.head_, nullptr);
        tail_ = std::exchange(other.tail_, nullptr);
        size_ = std::exchange(other.size_, 0);

        return *this;
    }

    template <typename T>
    T& list<T>::operator[](const size_t idx)
    {
        node* n = get_node(idx);
        assert(n);
        return n->data;
    }

    template <typename T>
    const T& list<T>::operator[](const size_t idx) const
    {
        const node* n = get_node(idx);
        assert(n);
        return n->data;
    }


    template <typename T>
    const list<T>::node* list<T>::get_node(size_t idx) const
    {
        if (idx >= size_) return tail_;
        if (idx == 0) return head_;

        node* n{ nullptr };

        if (idx < size_ / 2)
        {
            for (n = head_; n && idx; n = n->next, --idx) {}
        }
        else
        {
            idx = size_ - idx - 1;
            for (n = tail_; n && idx; n = n->prev, --idx) {}
        }

        return n;
    }

    template <typename T>
    void list<T>::reset()
    {
        for (node* n = head_; n; )
        {
            node* next = n->next;
            delete n;
            n = next;
        }

        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }


    template <typename T>
    template <typename... Args> requires std::constructible_from<T, Args...>
    void list<T>::emplace_front(Args&&... args)
    {
        node* new_node = new node{ nullptr, head_, std::forward<Args>(args)... };

        if (head_) head_->prev = new_node;
        else tail_ = new_node;

        head_ = new_node;
        ++size_;
    }

    template <typename T>
    template <typename... Args> requires std::constructible_from<T, Args...>
    void list<T>::emplace(const size_t idx, Args&&... args)
    {
        if (idx == 0) { emplace_front(std::forward<Args>(args)...); return; }
        if (idx >= size_) { emplace_back(std::forward<Args>(args)...); return; }

        node* n = get_node(idx - 1);
        node* new_node = new node{ n, n->next, std::forward<Args>(args)... };

        n->next = new_node;
        if (new_node->next) new_node->next->prev = new_node;

        ++size_;
    }

    template <typename T>
    template <typename... Args> requires std::constructible_from<T, Args...>
    void list<T>::emplace_back(Args&&... args)
    {
        node* new_node = new node{ tail_, nullptr, std::forward<Args>(args)... };

        if (tail_) tail_->next = new_node;
        else head_ = new_node;

        tail_ = new_node;
        ++size_;
    }


    template <typename T>
    void list<T>::pop_front()
    {
        if (!head_) return;

        node* new_head = head_->next;
        delete head_;

        head_ = new_head;

        if (head_) head_->prev = nullptr;
        else tail_ = nullptr;

        --size_;
    }

    template <typename T>
    void list<T>::pop(const size_t idx)
    {
        if (idx == 0) { pop_front(); return; }
        if (idx >= size_ - 1) { pop_back(); return; }

        node* n = head_;
        for (size_t i = 0; i < idx; ++i) n = n->next;

        node* prev = n->prev;
        node* next = n->next;

        delete n;

        if (prev) prev->next = next;
        if (next) next->prev = prev;

        --size_;
    }

    template <typename T>
    void list<T>::pop_back()
    {
        if (!tail_) return;

        node* new_tail = tail_->prev;
        delete tail_;

        tail_ = new_tail;

        if (tail_) tail_->next = nullptr;
        else head_ = nullptr;

        --size_;
    }

    template <typename T>
    void list<T>::swap_elements(node* n1, node* n2)
    {
        if (n1 == n2) return;

        if (n1->prev) n1->prev->next = n2;
        if (n1->next) n1->next->prev = n2;

        if (n2->prev) n2->prev->next = n1;
        if (n2->next) n2->next->prev = n1;

        std::swap(n1->prev, n2->prev);
        std::swap(n1->next, n2->next);
    }
}
