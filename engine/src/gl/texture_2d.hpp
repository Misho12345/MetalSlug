#pragma once
#include "mse/pch.hpp"

namespace mse::gl
{
    enum class TextureFormat
    {
        RGBA8
    };

    enum class TextureWrap
    {
        ClampToEdge,
        Repeat
    };

    enum class TextureFilter
    {
        Nearest,
        Linear
    };

    struct TextureDesc final
    {
        TextureFormat format{ TextureFormat::RGBA8 };
        TextureWrap   wrap{ TextureWrap::ClampToEdge };
        TextureFilter filter{ TextureFilter::Nearest };
    };

    class Texture2D final
    {
    public:
        Texture2D() = default;
        ~Texture2D() { reset(); }

        Texture2D(const Texture2D&)            = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;

        bool create(const TextureDesc& desc, vector<string> paths, bool force_array = false);
        void bind(GLuint unit) const;
        void bind_image(GLuint unit, GLenum access) const;

        void reset();

        glm::uvec3 size() const { return size_; }
        GLuint id() const { return id_; }
        operator bool() const { return id_; }

    private:
        TextureDesc desc_{};
        glm::uvec3  size_{ 0 };
        GLuint id_{ 0 };
    };
}