#pragma once

#include "mse/pch.hpp"

#include "gl/vao.hpp"
#include "gl/shader.hpp"
#include "gl/texture_2d.hpp"

#include "platform/window.hpp"

namespace mse
{
    struct PrivCtx;

    struct CrtSettings final
    {
        vec2  warp{ 0.031f, 0.041f };
        float hard_scan{ -8.0f };
        float hard_pix{ -8.0f };
        float mask_dark{ 0.5f };
        float mask_light{ 1.5f };
        float bright_boost{ 1.0f };
    };

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

        bool crt_enabled = true;

    private:
        PostProcessor() = default;

        gl::VAO vao;
        gl::Shader shader;
        gl::Texture2D color_tex;

        GLuint fbo = 0;

        GLint enabled_loc      = -1;
        GLint warp_loc         = -1;
        GLint hard_scan_loc    = -1;
        GLint hard_pix_loc     = -1;
        GLint mask_dark_loc    = -1;
        GLint mask_light_loc   = -1;
        GLint bright_boost_loc = -1;

        CrtSettings crt;

        friend ::mse::PrivCtx;
    };
}