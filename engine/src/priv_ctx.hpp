#pragma once
#include "renderer.hpp"
#include "sprite_manager.hpp"
#include "window.hpp"

namespace mse
{
    struct PrivCtx
    {
        Window window;

        SpriteManager sprite_manager;
        Renderer      renderer;
    };
}
