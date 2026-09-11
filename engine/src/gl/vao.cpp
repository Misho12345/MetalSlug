#include "mse/pch.hpp"
#include "vao.hpp"

namespace mse::gl
{
    VAO::VAO(VAO&& other) noexcept : id_{ std::exchange(other.id_, 0u) } {}

    VAO& VAO::operator=(VAO&& other) noexcept
    {
        if (this == &other) return *this;
        reset();
        id_ = std::exchange(other.id_, 0u);
        return *this;
    }

    void VAO::create(const vector<VertexAttrib>& attribs)
    {
        reset();
        glCreateVertexArrays(1, &id_);

        for (size_t i = 0; i < attribs.size(); ++i)
        {
            const VertexAttrib& a = attribs[i];
            glEnableVertexArrayAttrib(id_, a.attrib_idx);
            glVertexArrayAttribFormat(id_, a.attrib_idx, a.size, a.type, a.normalized, a.rel_offset);
            glVertexArrayAttribBinding(id_, a.attrib_idx, a.binding_idx);
        }
    }

    void VAO::reset()
    {
        if (id_) glDeleteVertexArrays(1, &id_);
        id_ = 0;
    }

    void VAO::bind_vbo(const Buffer& vbo, const GLsizei stride) const
    {
        glVertexArrayVertexBuffer(id_, 0, vbo.id(), 0, stride);
    }

    void VAO::bind_ebo(const Buffer& ebo) const
    {
        glVertexArrayElementBuffer(id_, ebo.id());
    }

    void VAO::bind() const { glBindVertexArray(id_); }
    void VAO::unbind() { glBindVertexArray(0);}
}
