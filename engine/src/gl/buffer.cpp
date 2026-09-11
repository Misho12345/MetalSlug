#include "mse/pch.hpp"
#include "buffer.hpp"

namespace mse::gl
{
    Buffer::Buffer(Buffer&& other) noexcept :
        type_{ std::exchange(other.type_, BufferType::Vertex) },
        id_{ std::exchange(other.id_, 0u) },
        mapped_ptr_{ std::exchange(other.mapped_ptr_, nullptr) },
        size_{ std::exchange(other.size_, 0u) } {}

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

    void Buffer::create(const BufferType type, const void* data, const GLsizeiptr size)
    {
        reset();
        glCreateBuffers(1, &id_);
        glNamedBufferData(id_, size, data, GL_STATIC_DRAW);

        type_ = type;
        size_ = size;
    }

    void Buffer::create_persistent_(const BufferType type, const GLsizeiptr size, const GLbitfield flags)
    {
        reset();

        type_ = type;
        size_ = size;
        flags_ = flags;

        glCreateBuffers(1, &id_);
        glNamedBufferStorage(id_, size, nullptr, flags);
        mapped_ptr_ = glMapNamedBufferRange(id_, 0, size, flags);
    }

    void Buffer::reset()
    {
        if (id_)
        {
            glDeleteBuffers(1, &id_);
            if (mapped_ptr_)
            {
                glUnmapNamedBuffer(id_);
                mapped_ptr_ = nullptr;
            }
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
            default: assert(false && "Unsupported buffer type for binding");
        }

        glBindBufferBase(target, binding, id_);
    }

    void Buffer::increase_size(const GLsizeiptr new_size)
    {
        if (new_size <= size_) return;

        const GLuint     old_id         = id_;
        const GLsizeiptr old_size       = size_;
        const void*      old_mapped_ptr = mapped_ptr_;

        id_         = 0;
        size_       = 0;
        mapped_ptr_ = nullptr;

        const GLsizeiptr copy_size = min(old_size, new_size);

        if (old_mapped_ptr)
        {
            create_persistent_(type_, new_size, flags_);
            std::memcpy(mapped_ptr_, old_mapped_ptr, copy_size);
        }
        else
        {
            create(type_, nullptr, new_size);
            glCopyNamedBufferSubData(old_id, id_, 0, 0, copy_size);
        }

        if (old_id)
        {
            if (old_mapped_ptr) glUnmapNamedBuffer(old_id);
            glDeleteBuffers(1, &old_id);
        }
    }
}
