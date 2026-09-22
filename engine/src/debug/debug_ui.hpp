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

        void draw_box(aabb box, glm::u8vec4 color) const;

    private:
        DebugUI() = default;

        friend ::mse::PrivCtx;
    };
}

#endif