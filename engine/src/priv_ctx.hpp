#pragma once

// prevent the executable from accessing the definition of PrivCtx
#ifndef MSE_EXPORTS
#error "You are not allowed to access the definition of PrivCtx"
#endif


#include "window.hpp"
#include "mse/input.hpp"

#include "physics_system.hpp"
#include "animation_system.hpp"
#include "rendering_system.hpp"

#include "post_processor.hpp"

#ifndef NDEBUG
#include "debug_ui.hpp"
#endif


namespace mse
{
    struct PrivCtx
    {
        Window window;

        // it's in here and not in GameCtx because I don't really want the user to access the instance,
        // but to just use the static interface
        Input input;

        PhysicsSystem   physics_system;
        AnimationSystem animation_system;
        RenderingSystem rendering_system;

        #ifndef NDEBUG
        DebugUI debug_ui;
        #endif

        PostProcessor post_processor;
    };
}
