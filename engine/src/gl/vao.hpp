#pragma once
#include "buffer.hpp"
#include "mse/pch.hpp"

namespace mse::gl
{
    namespace detail
    {
        template <typename T>
        constexpr GLenum get_gl_type() noexcept
        {
            if constexpr (std::same_as<T, int8_t>) return GL_BYTE;
            else if constexpr (std::same_as<T, uint8_t>) return GL_UNSIGNED_BYTE;
            else if constexpr (std::same_as<T, int16_t>) return GL_SHORT;
            else if constexpr (std::same_as<T, uint16_t>) return GL_UNSIGNED_SHORT;
            else if constexpr (std::same_as<T, int32_t>) return GL_INT;
            else if constexpr (std::same_as<T, uint32_t>) return GL_UNSIGNED_INT;
            else if constexpr (std::same_as<T, float>) return GL_FLOAT;
            else if constexpr (std::same_as<T, double>) return GL_DOUBLE;
            else static_assert(0, "Unsupported type for OpenGL");
        }
    }

    struct VertexAttrib final
    {
        template <typename M>
        static VertexAttrib make(
            const GLuint _rel_offset,
            const GLuint _attrib_idx,
            const GLuint _binding_idx = 0,
            const bool   _normalized = false)
        {
            if constexpr (requires { typename M::value_type; }) // glm
            {
                using T = M::value_type;

                return VertexAttrib{
                    .size        = sizeof(M) / sizeof(T),
                    .type        = detail::get_gl_type<T>(),
                    .rel_offset  = _rel_offset,
                    .attrib_idx  = _attrib_idx,
                    .binding_idx = _binding_idx,
                    .normalized  = _normalized
                };
            }
            else
            {
                return VertexAttrib{
                    .size        = 1,
                    .type        = detail::get_gl_type<M>(),
                    .rel_offset  = _rel_offset,
                    .attrib_idx  = _attrib_idx,
                    .binding_idx = _binding_idx,
                    .normalized  = _normalized
                };
            }
        }

        GLint  size;
        GLenum type;
        GLuint rel_offset;
        GLuint attrib_idx;
        GLuint binding_idx;
        bool normalized;
    };

    class VAO final
    {
    public:
        VAO() = default;
        ~VAO() { reset(); }

        VAO(const VAO&)            = delete;
        VAO& operator=(const VAO&) = delete;
        VAO(VAO&& other) noexcept;

        VAO& operator=(VAO&& other) noexcept;

        void create(const vector<VertexAttrib>& attribs);
        void reset();

        void bind_vbo(const Buffer& vbo, GLsizei stride) const;
        void bind_ebo(const Buffer& ebo) const;

        void bind() const;
        static void unbind();

        GLuint id() const { return id_; }
        operator bool() const { return id_; }

    private:
        GLuint id_{ 0 };
    };
}
