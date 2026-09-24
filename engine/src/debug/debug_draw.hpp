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
        static constexpr glm::u8vec4 HITBOX_COLOR{ 0, 255, 0, 255 };
        static constexpr glm::u8vec4 PIXEL_COLOR{ 0, 200, 200, 255 };

        static constexpr glm::u8vec4 tile_color(const TileType type)
        {
            switch (type)
            {
                case TileType::Floor: return glm::u8vec4{ 255, 0, 0, 255 };
                default: assert(false && "not implemented or air");
            }

            return {};
        }

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
        void draw_tile_map() const;

        bool active_{ false };

        friend ::mse::PrivCtx;
    };
}
#endif
