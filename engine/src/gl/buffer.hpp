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
            create(type, data.data(), static_cast<GLsizeiptr>(data.size() * sizeof(T)));
        }

        /**
         * @brief Creates a persistent buffer of the specified type and size.
         * @tparam T The type of the data to be stored in the buffer.
         * @param type The type of the buffer.
         * @param count The number of elements in the buffer.
         * @param flags Buffer mapping flags.
         * @note
         * The buffer is persistent and can be mapped for writing.
         */
        template <typename T>
        void create_persistent(
            const BufferType type,
            const GLsizeiptr count,
            const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
        {
            create_persistent_(type, count * sizeof(T), flags);
        }

        void reset();

        void bind(GLuint binding) const;

        template <typename T>
        T* mapped_data() const { return static_cast<T*>(mapped_ptr_); }

        /**
         * @brief Increases the size of the buffer to the specified new size.
         * @note Allocates a new buffer with the specified new size and
         * copies the existing data to the new buffer if resize is needed.
         */
        void increase_size(GLsizeiptr new_size);

        GLuint id() const { return id_; }

        template <typename T>
        GLsizeiptr size() const { return size_ / sizeof(T); }

        operator bool() const { return id_; }

    private:
        void create(BufferType type, const void* data, GLsizeiptr size);

        void create_persistent_(
            BufferType type,
            GLsizeiptr size,
            GLbitfield flags);

        BufferType type_{ BufferType::Vertex };
        GLuint     id_{ 0 };

        GLsizeiptr size_{ 0 };
        GLbitfield flags_{ 0 };

        void* mapped_ptr_{ nullptr };
    };
}
