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

        priv_ctx_->input.init();

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            printf("Failed to initialize GLAD\n");
            ok_ = false;
            return;
        }

        priv_ctx_->rendering_system.init_global();

        if (!priv_ctx_->animation_system.init())
        {
            printf("failed to init animation system\n");
            ok_ = false;
            return;
        }


        #ifndef NDEBUG
        priv_ctx_->debug_ui.init();
        #endif

       if (!priv_ctx_->post_processor.init(
            "assets/shaders/post_processing.vert",
            "assets/shaders/post_processing.frag"))
        {
            printf("failed to init post processor\n");
            ok_ = false;
            return;
        }

        printf("finished setup\n");
    }

    void App::run()
    {
        if (!ok_) return;

        // here and not in setup() because it has to have the animation frame counts set in App::init()
        if (!priv_ctx_->rendering_system.init_sprite_objects(ctx_->scene))
        {
            printf("failed to init sprite objects\n");
            return;
        }

        float time = static_cast<float>(glfwGetTime());

        while (!priv_ctx_->window.should_close())
        {
            priv_ctx_->input.update();
            priv_ctx_->window.poll_events();

            #ifndef NDEBUG
            priv_ctx_->debug_ui.begin_frame();
            #endif

            const float new_time = static_cast<float>(glfwGetTime());
            const float dt = new_time - time;
            time = new_time;

            update(dt);

            static float accumulator = 0.0f;
            accumulator += dt;

            static bool debug_draw = false;
            if (Input::down(Key::Ctrl) && Input::just_pressed(Key::B)) debug_draw = !debug_draw;

            uint32_t c = PhysicsSystem::MAX_STEPS;
            while (c && accumulator > PhysicsSystem::FIXED_TIME_STEP)
            {
                fixed_update();
                priv_ctx_->physics_system.step(ctx_->scene, debug_draw);
                accumulator -= PhysicsSystem::FIXED_TIME_STEP;
                --c;
            }

            priv_ctx_->animation_system.update(ctx_->scene, dt);

            priv_ctx_->post_processor.bind();
            priv_ctx_->rendering_system.render_sprites(ctx_->scene);

            priv_ctx_->post_processor.render(priv_ctx_->window);

            #ifndef NDEBUG
            priv_ctx_->debug_ui.render();
            #endif

            priv_ctx_->window.swap_buffers();
        }
    }

    void App::shutdown()
    {
        printf("MSE shutdown\n");
    }
}
