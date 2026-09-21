#pragma once

#include <concepts>
#include <utility>

#include <cassert>

namespace mse
{
    /// @brief Iterator for the list
    template <typename T, typename N>
    struct list_iterator
    {
        N ptr_{ nullptr };

        T& operator*() const { return ptr_->data; }
        T* operator->() const { return &ptr_->data; }

        list_iterator& operator++()
        {
            ptr_ = ptr_->next;
            return *this;
        }

        list_iterator operator++(int)
        {
            list_iterator tmp = *this;
            ptr_ = ptr_->next;
            return tmp;
        }

        list_iterator& operator--()
        {
            ptr_ = ptr_->prev;
            return *this;
        }

        list_iterator operator--(int)
        {
            list_iterator tmp = *this;
            ptr_ = ptr_->prev;
            return tmp;
        }

        bool operator==(const list_iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const list_iterator& other) const { return ptr_ != other.ptr_; }
    };

    /**
     * @brief A doubly linked list implementation
     * @tparam T The type of the elements in the list
     */
    template <typename T>
    class list final
    {
    public:
        /// @brief Node definition for list
        struct node final
        {
            template <typename... Args> requires std::constructible_from<T, Args...>
            node(node* _prev, node* _next, Args&&... args)
                : prev{ _prev },
                  next{ _next },
                  data{ std::forward<Args>(args)... } {}

            node* prev{ nullptr };
            node* next{ nullptr };
            T data;
        };

        /**
         * @brief Constructs a list with the given elements
         * @tparam Args The types of the elements
         * @param args The elements to add to the list
         *
         * Example:
         * @code
         * // constructs a list with the elements 1.0f, 2.0f, 5.0f
         * list<float> l{ 1, 2.0f, 5.5f };
         * @endcode
         */
        template <typename... Args> requires (std::convertible_to<Args, T> && ...)
        list(Args&&... args);

        ~list();

        list(const list& other);
        list(list&& other) noexcept;
        list& operator=(const list& other);
        list& operator=(list&& other) noexcept;


        /**
         * @brief Accesses the element at the specified index
         * @param idx Index of the element to access
         * @return Reference to the element at the specified index; if idx is out of bounds the value in tail_ is returned.
         * @note
         * In case a node is accessed that does not exist
         * (for example the list is empty), assert will be triggered
         */
        [[nodiscard]]
        T& operator[](size_t idx);

        /**
         * @brief Accesses the element at the specified index
         * @param idx Index of the element to access
         * @return Const reference to the element at the specified index; if idx is out of bounds the value in tail_ is returned.
         * @note
         * In case a node is accessed that does not exist
         * (for example the list is empty), assert will be triggered
         */
        [[nodiscard]]
        const T& operator[](size_t idx) const;

        /**
         * @brief Accesses the first element in the list
         * @return Reference to the first element in the list
         * @note If the list is empty, assert will be triggered
         */
        [[nodiscard]]
        T& front() { assert(head_); return head_->data; }

        /**
         * @brief Accesses the first element in the list
         * @return Const reference to the first element in the list
         * @note If the list is empty, assert will be triggered
         */
        [[nodiscard]]
        const T& front() const { assert(head_); return head_->data; }

        /**
         * @brief Accesses the last element in the list
         * @return Reference to the last element in the list
         * @note If the list is empty, assert will be triggered
         */
        [[nodiscard]]
        T& back() { assert(tail_); return tail_->data; }

        /**
         * @brief Accesses the last element in the list
         * @return Const reference to the last element in the list
         * @note If the list is empty, assert will be triggered
         */
        [[nodiscard]]
        const T& back() const { assert(tail_); return tail_->data; }


        [[nodiscard]] node* head() { return head_; }
        [[nodiscard]] const node* head() const { return head_; }

        [[nodiscard]] node* tail() { return tail_; }
        [[nodiscard]] const node* tail() const { return tail_; }


        /**
         * @brief Accesses the element at the specified index
         * @param idx Index of the element to access
         * @return Node of the element at the specified index; if idx is out of bounds tail_ is returned.
         */
        [[nodiscard]]
        node* get_node(const size_t idx)
        {
            return const_cast<node*>(std::as_const(*this).get_node(idx));
        }

        /**
         * @brief Accesses the element at the specified index
         * @param idx Index of the element to access
         * @return Node of the element at the specified index; if idx is out of bounds tail_ is returned.
         */
        [[nodiscard]]
        const node* get_node(size_t idx) const;


        void reset();
        [[nodiscard]] bool empty() const { return !size_; }
        [[nodiscard]] size_t size() const { return size_; }



        /// @brief Inserts a copy of the specified value to the front of the list
        void push_front(const T& value) { emplace_front(value); }

        /// @brief Inserts a copy of the specified value to the list at the specified index
        void push(size_t idx, const T& value) { emplace(idx, value); }

        /// @brief Inserts a copy of the specified value to the back of the list
        void push_back(const T& value) { emplace_back(value); }



        /// @brief Moves the specified value to the front of the list
        void push_front(T&& value) { emplace_front(std::move(value)); }

        /// @brief Moves the specified value to the list at the specified index
        void push(size_t idx, T&& value) { emplace(idx, std::move(value)); }

        /// @brief Moves the specified value to the back of the list
        void push_back(T&& value) { emplace_back(std::move(value)); }


        /**
         * @brief Constructs a new element in-place at the front of the list
         * @tparam Args Arguments to construct the new element
         * @param args Arguments to construct the new element
         */
        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace_front(Args&&... args);

        /**
         * @brief Constructs a new element in-place at the specified index of the list
         * @tparam Args Arguments to construct the new element
         * @param idx Index to insert the new element at
         * @param args Arguments to construct the new element
         */
        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace(size_t idx, Args&&... args);


        /**
         * @brief Constructs a new element in-place at the back of the list
         * @tparam Args Arguments to construct the new element
         * @param args Arguments to construct the new element
         */
        template <typename... Args> requires std::constructible_from<T, Args...>
        void emplace_back(Args&&... args);


        void pop_front();
        void pop(size_t idx);
        void pop_back();

        /**
         * @brief Swaps the specified elements in the list
         * @param n1 First element to swap
         * @param n2 Second element to swap
         * @note
         * The function doesn't ensure that the elements are valid or that they belong
         * to the same list; it just swaps the pointers for next and prev.
         * It does ensure that head_ and tail_ remain accurate after the swap.
         */
        void swap_elements(node* n1, node* n2);


        // iterator methods
        using iterator       = list_iterator<T, node*>;
        using const_iterator = list_iterator<const T, const node*>;

        iterator begin() { return iterator{ head_ }; }
        iterator end()   { return iterator{ nullptr }; }

        const_iterator begin() const { return const_iterator{ head_ }; }
        const_iterator end()   const { return const_iterator{ nullptr }; }

        const_iterator cbegin() const { return begin(); }
        const_iterator cend()   const { return end(); }

    private:
        node* head_{ nullptr };
        node* tail_{ nullptr };
        size_t size_{ 0 };
    };
}

#include "list.inl"