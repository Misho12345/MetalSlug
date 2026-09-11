#include "mse/pch.hpp"
#include "texture_2d.hpp"

namespace mse::gl
{
    namespace
    {
        GLenum to_gl(const TextureFormat format) noexcept
        {
            switch (format)
            {
                case TextureFormat::RGBA8: return GL_RGBA8;
            }

            assert(false);
        }

        GLenum to_gl_source_fmt(const TextureFormat format) noexcept
        {
            switch (format)
            {
                case TextureFormat::RGBA8: return GL_RGBA;
            }

            assert(false);
        }

        GLint to_gl(const TextureFilter filter) noexcept
        {
            switch (filter)
            {
                case TextureFilter::Nearest: return GL_NEAREST;
                case TextureFilter::Linear: return GL_LINEAR;
            }

            assert(false);
        }

        GLint to_gl(const TextureWrap wrap) noexcept
        {
            switch (wrap)
            {
                case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
                case TextureWrap::Repeat: return GL_REPEAT;
            }

            assert(false);
        }

        int format_size(const TextureFormat format) noexcept
        {
            switch (format)
            {
                case TextureFormat::RGBA8: return 4;
            }

            assert(false);
        }
    }

    Texture2D::Texture2D(Texture2D&& other) noexcept :
        desc_{ std::exchange(other.desc_, TextureDesc{}) },
        size_{ std::exchange(other.size_, glm::uvec3{}) },
        id_{ std::exchange(other.id_, 0u) } {}

    Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
    {
        if (this == &other) return *this;
        reset();

        desc_ = std::exchange(other.desc_, TextureDesc{});
        size_ = std::exchange(other.size_, glm::uvec3{});
        id_ = std::exchange(other.id_, 0u);

        return *this;
    }

    bool Texture2D::create(const TextureDesc& desc, vector<string> paths, const bool force_array)
    {
        if (paths.empty()) return false;

        reset();

        int w, h, c;
        stbi_info(paths.front().data(), &w, &h, &c);
        size_ = { w, h, paths.size() };
        const bool array = size_.z > 1 || force_array;

        if (array)
        {
            glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id_);
            glTextureStorage3D(id_, 1, to_gl(desc.format), w, h, size_.z);
        }
        else
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &id_);
            glTextureStorage2D(id_, 1, to_gl(desc.format), w, h);
        }

        glTextureParameteri(id_, GL_TEXTURE_WRAP_S, to_gl(desc.wrap));
        glTextureParameteri(id_, GL_TEXTURE_WRAP_T, to_gl(desc.wrap));
        glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, to_gl(desc.filter));
        glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, to_gl(desc.filter));

        for (size_t i = 0; i < paths.size(); ++i)
        {
            const string_view path = paths[i];

            int w_, h_, c_;
            uint8_t* pixels = stbi_load(path.data(), &w_, &h_, &c_, format_size(desc.format));

            if (w_ != w || h_ != h)
            {
                printf("cannot have texture 2d array of textures of different sizes!");
                return false;
            }

            if (array)
            {
                glTextureSubImage3D(
                    id_,
                    0, 0, 0, static_cast<GLint>(i),
                    w, h, 1,
                    to_gl_source_fmt(desc.format),
                    GL_UNSIGNED_BYTE,
                    pixels);
            }
            else
            {
                glTextureSubImage2D(
                    id_,
                    0, 0, 0,
                    w, h,
                    to_gl_source_fmt(desc.format),
                    GL_UNSIGNED_BYTE,
                    pixels);
            }

            stbi_image_free(pixels);
        }

        desc_ = desc;
        return true;
    }

    void Texture2D::bind(const GLuint unit) const
    {
        if (!id_) return;
        glBindTextureUnit(unit, id_);
    }

    void Texture2D::bind_image(const GLuint unit, const GLenum access) const
    {
        if (!id_) return;
        glBindImageTexture(unit, id_, 0, GL_FALSE, 0, access, to_gl(desc_.format));
    }

    void Texture2D::reset()
    {
        if (id_) glDeleteTextures(1, &id_);
        id_ = 0;
    }
}
