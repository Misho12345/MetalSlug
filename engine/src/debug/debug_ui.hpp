#pragma once
#include "mse/collision/aabb.hpp"
#include "mse/pch.hpp"

#ifndef NDEBUG

namespace mse
{
    struct PrivCtx;

    /**
     * @brief Debug UI class that wraps around imgui
     * @details It exists only for debugging purposes and exists only in debug mode
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class DebugUI final
    {
    public:
        ~DebugUI();

        DebugUI(const DebugUI&)            = delete;
        DebugUI(DebugUI&&)                 = delete;
        DebugUI& operator=(const DebugUI&) = delete;
        DebugUI& operator=(DebugUI&&)      = delete;

        void init() const;
        void begin_frame() const;
        void render() const;

        void draw_box(aabb box, color color, float thickness = 2.0f) const;
        void draw_line(ivec2 start, ivec2 end, color color, float thickness = 2.0f) const;

    private:
        DebugUI() = default;

        static void get_screen_transform(vec2& out_scale, vec2& out_offset);

        friend ::mse::PrivCtx;
    };
}

#endif