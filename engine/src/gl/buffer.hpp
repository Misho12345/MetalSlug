#pragma once
#include "mse/pch.hpp"

namespace mse::gl
{
    enum class BufferType
    {
        Vertex,
        Index,
        Uniform,
        Storage,
    };

    /**
     * @brief The Buffer class encapsulates an OpenGL buffer
     * @details It provides functionality to create, bind, and manage OpenGL buffers.
     */
    class Buffer final
    {
    public:
        Buffer() = default;
        ~Buffer() { reset(); }

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        /**
         * @brief Creates a buffer of the specified type and initializes it with the provided data.
         * @tparam T The type of the data to be stored in the buffer.
         * @param type The type of the buffer.
         * @param data A span containing the data to be stored in the buffer.
         * @note
         * Do not try and use mapped_data() buffer for reading or writing if the buffer is not persistent.
         */
        template <typename T>
        void create(const BufferType type, span<const T> data)
        {
            create(type, data.data(), data.size() * sizeof(T));
        }

        /**
         * @brief Creates a persistent buffer of the specified type and size.
         * @tparam T The type of the data to be stored in the buffer.
         * @param type The type of the buffer.
         * @param count The number of elements in the buffer.
         * @param flags Buffer mapping flags.
         * @note
         * The buffer is persistent and can be mapped for writing.
         * This is for an immutable persistent buffer, I'm too lazy to implement ring buffering,
         * so for changing UBOs a normal create() + write() is what should be used
         */
        template <typename T>
        void create_persistent(
            const BufferType type,
            const size_t     count,
            const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
        {
            create_persistent_(type, count * sizeof(T), flags);
        }


        void reset();

        void bind(GLuint binding) const;


        /**
         * @brief Writes data to the buffer
         * @tparam T The type of an element in the data
         * @param data A span containing the data to be stored in the buffer.
         */
        template <typename T>
        void write(span<const T> data) const { write(data.data(), data.size() * sizeof(T)); }

        template <typename T>
        [[nodiscard]]
        T* mapped_data() const { return static_cast<T*>(mapped_ptr_); }

        /**
         * @brief Increases the size of the buffer to the specified new size.
         * @note Allocates a new buffer with the specified new size and
         * copies the existing data to the new buffer if resize is needed.
         */
        template <typename T>
        void increase_size(const size_t new_size) { increase_size_(new_size * sizeof(T)); }

        [[nodiscard]]
        GLuint id() const { return id_; }

        template <typename T>
        [[nodiscard]]
        size_t size() const { return size_ / sizeof(T); }

        operator bool() const { return id_; }

    private:
        void increase_size_(size_t new_size);

        void create(BufferType type, const void* data, size_t size);

        void create_persistent_(
            BufferType type,
            size_t     size,
            GLbitfield flags);

        void write(const void* data, size_t size) const;

        BufferType type_{ BufferType::Vertex };
        GLuint     id_{ 0 };

        size_t     size_{ 0 };
        GLbitfield flags_{ 0 };

        void* mapped_ptr_{ nullptr };
    };
}
