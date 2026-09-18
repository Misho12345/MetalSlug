#include "mse/pch.hpp"
#include "post_processor.hpp"

namespace mse
{
    PostProcessor::~PostProcessor()
    {
        glDeleteFramebuffers(1, &fbo);
    }

    void PostProcessor::init(const string_view vertex_path, const string_view fragment_path)
    {
        vao.create({});
        shader.create(vertex_path, fragment_path);

        color_tex.create({ Target::RESOLUTION, 1 }, {
            .format = gl::TextureFormat::RGB8,
            .wrap = gl::TextureWrap::ClampToEdge,
            .filter = gl::TextureFilter::Nearest
        });

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex.id(), 0);
    }

    void PostProcessor::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, Target::RESOLUTION.x, Target::RESOLUTION.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void PostProcessor::render(const Window& window) const
    {
        const float ar = window.aspect_ratio();

        glm::uvec2 offset{}, size;

        if (ar > Target::DAR)
        {
            size.x = window.height() * Target::DAR;
            size.y = window.height();
            offset.x = (window.width() - size.x) * 0.5f;
        }
        else
        {
            size.x = window.width();
            size.y = window.width() / Target::DAR;
            offset.y = (window.height() - size.y) * 0.5f;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, window.width(), window.height());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(offset.x, offset.y, size.x, size.y);

        shader.use();
        vao.bind();
        color_tex.bind(0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
