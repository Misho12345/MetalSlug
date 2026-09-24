#include "mse/pch.hpp"

#include "mse/app.hpp"
#include "mse/input/input.hpp"
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

        if (!priv_ctx_->sprite_data_registry.init())
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

        if (!priv_ctx_->tile_map.load("assets/tile_maps/mission1.csv"))
        {
            printf("couldn't load tile map\n");
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

        double time = glfwGetTime();

        while (!priv_ctx_->window.should_close())
        {
            priv_ctx_->window.poll_events();

            const double new_time = glfwGetTime();
            accumulator_ += min(new_time - time, MAX_DELTA_TIME);
            time = new_time;


            #ifndef NDEBUG
            priv_ctx_->debug_ui.begin_frame();
            #endif

            priv_ctx_->post_processor.bind();

            uint32_t c = 0;

            while (accumulator_ >= Target::FRAME_TIME)
            {
                update();
                priv_ctx_->animation_system.update(ctx_->scene);
                priv_ctx_->collision_system.step(ctx_->scene);
                accumulator_ -= Target::FRAME_TIME;

                if (c == 0)
                {
                    #ifndef NDEBUG
                    if (Input::down(Key::Ctrl) && Input::just_pressed(Key::B))
                        priv_ctx_->debug_draw.toggle();
                    #endif

                    priv_ctx_->input.clear();
                }

                if (++c >= MAX_STEPS)
                {
                    accumulator_ = 0;
                    break;
                }
            }

            priv_ctx_->rendering_system.render_sprites(ctx_->scene);
            priv_ctx_->post_processor.render(priv_ctx_->window);

            #ifndef NDEBUG
            priv_ctx_->debug_draw.draw(ctx_->scene);
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
