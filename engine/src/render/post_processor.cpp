#include "mse/pch.hpp"
#include "post_processor.hpp"

namespace mse
{
    PostProcessor::~PostProcessor()
    {
        glDeleteFramebuffers(1, &fbo);
    }

    bool PostProcessor::init(const string_view vertex_path, const string_view fragment_path)
    {
        vao.create({});

        if (!shader.create(vertex_path, fragment_path))
        {
            printf("failed to create post processing shader");
            return false;
        }

        color_tex.create(
            { Target::RESOLUTION, 1 }, {
                .format = gl::TextureFormat::RGB8,
                .wrap   = gl::TextureWrap::ClampToEdge,
                .filter = gl::TextureFilter::Nearest
            });

        enabled_loc      = shader.uniform_location("u_Enabled");
        warp_loc         = shader.uniform_location("u_Warp");
        hard_scan_loc    = shader.uniform_location("u_HardScan");
        hard_pix_loc     = shader.uniform_location("u_HardPix");
        mask_dark_loc    = shader.uniform_location("u_MaskDark");
        mask_light_loc   = shader.uniform_location("u_MaskLight");
        bright_boost_loc = shader.uniform_location("u_BrightBoost");

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex.id(), 0);

        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    void PostProcessor::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, RESOLUTION_i.x, RESOLUTION_i.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void PostProcessor::render(const Window& window) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(
            0, 0,
            (GLsizei)window.width(),
            (GLsizei)window.height());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        const ivec2 offset = window.output_offset();
        const ivec2 size   = window.output_size();
        glViewport(offset.x, offset.y, size.x, size.y);


        shader.use();
        gl::Shader::set_uniform(enabled_loc, (int32_t)crt_enabled);
        gl::Shader::set_uniform(warp_loc,             crt.warp);
        gl::Shader::set_uniform(hard_scan_loc,        crt.hard_scan);
        gl::Shader::set_uniform(hard_pix_loc,         crt.hard_pix);
        gl::Shader::set_uniform(mask_dark_loc,        crt.mask_dark);
        gl::Shader::set_uniform(mask_light_loc,       crt.mask_light);
        gl::Shader::set_uniform(bright_boost_loc,     crt.bright_boost);

        vao.bind();
        color_tex.bind(0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
