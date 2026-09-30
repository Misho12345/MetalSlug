#include "mse/pch.hpp"
#include "buffer.hpp"

namespace mse::gl
{
    Buffer::Buffer(Buffer&& other) noexcept :
        type_{ std::exchange(other.type_, BufferType::Vertex) },
        id_{ std::exchange(other.id_, 0u) },
        size_{ std::exchange(other.size_, 0u) },
        mapped_ptr_{ std::exchange(other.mapped_ptr_, nullptr) } {}

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        if (this == &other) return *this;
        reset();

        type_       = std::exchange(other.type_, BufferType::Vertex);
        id_         = std::exchange(other.id_, 0u);
        mapped_ptr_ = std::exchange(other.mapped_ptr_, nullptr);
        size_       = std::exchange(other.size_, 0u);

        return *this;
    }

    void Buffer::create(const BufferType type, const void* data, const size_t size)
    {
        assert(data && "data pointer is nullptr");
        assert(size > 0 && "buffer size is 0");

        reset();
        glCreateBuffers(1, &id_);
        glNamedBufferData(id_, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);

        type_ = type;
        size_ = size;
    }

    void Buffer::create_persistent_(const BufferType type, const size_t size, const GLbitfield flags)
    {
        assert(size > 0 && "buffer size is 0");
        reset();

        type_ = type;
        size_ = size;
        flags_ = flags;

        glCreateBuffers(1, &id_);
        glNamedBufferStorage(id_, static_cast<GLsizeiptr>(size), nullptr, flags);
        mapped_ptr_ = glMapNamedBufferRange(id_, 0, static_cast<GLsizeiptr>(size), flags);
    }

    void Buffer::write(const void* data, const size_t size) const
    {
        assert(id_ && "invalid buffer");
        assert(data && "data pointer is nullptr");
        assert(size > 0 && "data size is zero");
        assert(size <= size_ && "data size exceeds buffer size");

        if (mapped_ptr_) memcpy(mapped_ptr_, data, size);
        else glNamedBufferSubData(id_, 0, static_cast<GLsizeiptr>(size), data);
    }

    void Buffer::reset()
    {
        if (id_)
        {
            if (mapped_ptr_)
            {
                glUnmapNamedBuffer(id_);
                mapped_ptr_ = nullptr;
            }
            glDeleteBuffers(1, &id_);
        }

        id_ = 0;
        size_ = 0;
    }

    void Buffer::bind(const GLuint binding) const
    {
        GLenum target;

        switch (type_)
        {
            case BufferType::Uniform: target = GL_UNIFORM_BUFFER; break;
            case BufferType::Storage: target = GL_SHADER_STORAGE_BUFFER; break;
            default:
                // you cannot bind vertex and instance buffers
                assert(false && "Unsupported buffer type for binding");
                target = {};
        }

        glBindBufferBase(target, binding, id_);
    }

    void Buffer::increase_size_(const size_t new_size)
    {
        if (new_size <= size_) return;

        const GLuint old_id         = id_;
        const size_t old_size       = size_;
        const void*  old_mapped_ptr = mapped_ptr_;

        id_         = 0;
        size_       = 0;
        mapped_ptr_ = nullptr;

        const size_t copy_size = min(old_size, new_size);

        if (old_mapped_ptr)
        {
            create_persistent_(type_, new_size, flags_);
            memcpy(mapped_ptr_, old_mapped_ptr, copy_size);
        }
        else
        {
            create(type_, nullptr, new_size);
            glCopyNamedBufferSubData(old_id, id_, 0, 0, static_cast<GLsizeiptr>(copy_size));
        }

        if (old_id)
        {
            if (old_mapped_ptr) glUnmapNamedBuffer(old_id);
            glDeleteBuffers(1, &old_id);
        }
    }
}
