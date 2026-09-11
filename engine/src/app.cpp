#include "mse/pch.hpp"

#include "mse/app.hpp"
#include "mse/input.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    App::App() :
        ctx_{ make_unique<GameCtx>() },
        priv_ctx_{ make_unique<PrivCtx>() } { instance_ = this; }

    App::~App() = default;

    void App::setup()
    {
        printf("MSE setup\n");

        if (!priv_ctx_->window.init({
            .width  = 1200,
            .height = 800,
            .title  = "MSE test"
        }))
        {
            ok_ = false;
            return;
        }

        Input::init();

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            printf("Failed to initialize GLAD\n");
            ok_ = false;
            return;
        }

        if (!priv_ctx_->sprite_manager.init())
        {
            printf("failed to init sprite manager\n");
            ok_ = false;
            return;
        }

        printf("finished setup\n");
    }

    void App::run()
    {
        if (!ok_) return;

        glViewport(0, 0, priv_ctx_->window.width(), priv_ctx_->window.height());

        if (!priv_ctx_->renderer.init_sprite_objects())
        {
            printf("failed to init sprite objects\n");
            return;
        }

        double time = glfwGetTime();

        while (!priv_ctx_->window.should_close())
        {
            Input::update();
            priv_ctx_->window.poll_events();

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            const float new_time = glfwGetTime();
            const float dt = static_cast<float>(new_time - time);
            time = new_time;

            priv_ctx_->sprite_manager.update(dt);
            priv_ctx_->renderer.render_sprites();

            priv_ctx_->window.swap_buffers();
            tick(dt);
        }
    }

    void App::shutdown()
    {
        printf("MSE shutdown\n");
    }
}
