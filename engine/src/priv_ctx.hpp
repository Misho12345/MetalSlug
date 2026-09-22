#pragma once

// prevent the executable from accessing the definition of PrivCtx
#ifndef MSE_EXPORTS
#error "You are not allowed to access the definition of PrivCtx"
#endif


#include "platform/window.hpp"

#include "mse/input/input.hpp"

#include "collision/tile_map.hpp"
#include "collision/collision_system.hpp"

#include "render/animation_system.hpp"
#include "render/rendering_system.hpp"

#ifndef NDEBUG
#include "debug/debug_ui.hpp"
#include "debug/debug_draw.hpp"
#endif

#include "render/post_processor.hpp"


namespace mse
{
    struct PrivCtx
    {
        Window window;

        // it's in here and not in GameCtx because I don't really want the user to access the instance,
        // but to just use the static interface
        Input input;

        TileMap         tile_map;
        CollisionSystem physics_system;

        AnimationSystem animation_system;
        RenderingSystem rendering_system;

        #ifndef NDEBUG
        DebugUI debug_ui;
        DebugDraw debug_draw;
        #endif

        PostProcessor post_processor;
    };
}
