#pragma once
#include "mse/pch.hpp"

namespace mse::gl
{
    enum class TextureFormat { RGB8, RGBA8 };
    enum class TextureWrap { ClampToEdge,Repeat };
    enum class TextureFilter { Nearest, Linear };

    struct TextureDesc final
    {
        TextureFormat format{ TextureFormat::RGBA8 };
        TextureWrap   wrap{ TextureWrap::ClampToEdge };
        TextureFilter filter{ TextureFilter::Nearest };
    };

    /// @brief Represents a 2D texture or 2d texture array in OpenGL
    class Texture2D final
    {
    public:
        Texture2D() = default;
        ~Texture2D() { reset(); }

        Texture2D(const Texture2D&)            = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;

        /**
         * @brief Creates an empty texture 2d (array) from the given size and description
         * @param size The size of the texture
         * @param desc The texture description
         * @param force_array Whether to force the texture to be an array
         * @note Texture 2D array will be created unless the number of paths is 1 and force_array is false
         */
        void create(glm::uvec3 size, const TextureDesc& desc, bool force_array = false);

        /**
         * @brief Creates a texture 2d (array) from the given paths
         * @param desc The texture description
         * @param paths The paths to the texture files
         * @param force_array Whether to force the texture to be an array
         * @return True if the texture was created successfully, false otherwise
         * @note Texture 2D array will be created unless the number of paths is 1 and force_array is false
         */
        bool create(const TextureDesc& desc, span<const string> paths, bool force_array = false);

        /**
         * @brief Binds the texture to the given texture unit as a sampler
         * @param unit The texture unit to bind the texture to
         */
        void bind(GLuint unit) const;

        /**
         * @brief Binds the texture to the given texture unit as an image
         * @param unit The texture unit to bind the texture to
         * @param access The access mode for the image
         */
        void bind_image(GLuint unit, GLenum access) const;

        void reset();

        glm::uvec3 size() const { return size_; } // size_.z is the number of layers in the texture array
        GLuint id() const { return id_; }
        operator bool() const { return id_; }

    private:
        TextureDesc desc_{};
        glm::uvec3  size_{ 0 };
        GLuint id_{ 0 };
    };
}