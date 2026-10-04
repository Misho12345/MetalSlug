#pragma once
#include "collision/tile_map.hpp"
#include "mse/ecs/scene.hpp"

#ifndef NDEBUG
namespace mse
{
    struct PrivCtx;

    /**
     * @brief Draws bounding boxes and tilemap collider
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class DebugDraw final
    {
    public:
        DebugDraw(const DebugDraw&)            = delete;
        DebugDraw(DebugDraw&&)                 = delete;
        DebugDraw& operator=(const DebugDraw&) = delete;
        DebugDraw& operator=(DebugDraw&&)      = delete;

        void toggle();
        void draw(const Scene& scene) const;

    private:
        DebugDraw() = default;

        void draw_colliders(const Scene& scene) const;
        void draw_tile_map(const Scene& scene) const;

        bool active_{ true };

        friend ::mse::PrivCtx;
    };
}
#endif
