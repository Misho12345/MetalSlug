#pragma once

#include "mse/pch.hpp"

#include "gl/vao.hpp"
#include "gl/shader.hpp"
#include "gl/texture_2d.hpp"

#include "platform/window.hpp"

namespace mse
{
    struct PrivCtx;

    /**
     * @brief Handles post processing and frame buffer management
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class PostProcessor final
    {
    public:
        ~PostProcessor();

        PostProcessor(const PostProcessor&)            = delete;
        PostProcessor(PostProcessor&&)                 = delete;
        PostProcessor& operator=(const PostProcessor&) = delete;
        PostProcessor& operator=(PostProcessor&&)      = delete;

        [[nodiscard]]
        bool init(string_view vertex_path, string_view fragment_path);
        void bind() const;
        void render(const Window& window) const;

    private:
        PostProcessor() = default;

        gl::VAO vao;
        gl::Shader shader;
        gl::Texture2D color_tex;

        GLuint fbo = 0;

        friend ::mse::PrivCtx;
    };
}