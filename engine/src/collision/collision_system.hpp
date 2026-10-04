#pragma once

#include "mse/app.hpp"
#include "mse/collision/aabb.hpp"

namespace mse
{
    class Scene;
    struct PrivCtx;

    using fixed_update_t = void(App::*)();

    /**
     * @brief Collision system
     * @details This system is responsible for updating the sprite collision components
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class CollisionSystem final
    {
    public:
        static constexpr float GRAVITY = 9.81f;
        static constexpr uint32_t X_GRID_CELL_SIZE = Target::TILE_SIZE.x * 4u;
        static constexpr uint32_t X_GRID_SIZE = (Target::RESOLUTION.x + X_GRID_CELL_SIZE - 1u) / X_GRID_CELL_SIZE; // ceil

        CollisionSystem(const CollisionSystem&)            = delete;
        CollisionSystem(CollisionSystem&&)                 = delete;
        CollisionSystem& operator=(const CollisionSystem&) = delete;
        CollisionSystem& operator=(CollisionSystem&&)      = delete;

        void step(Scene& scene);

    private:
        struct EntityExtent final
        {
            entity_id entity{};
            uvec2 range{};
        };

        struct CollisionPair final
        {
            entity_id a{};
            entity_id b{};
        };

        struct BucketEntry final
        {
            entity_id entity{};
            uint32_t starting_grid_id{};
        };

        CollisionSystem();

        static bool collision_check(Scene& scene, entity_id a, entity_id b);

        // performs a pixel perfect check using
        static bool pp_check(const Scene& scene, entity_id a, entity_id b);


        static void clamp_move_to_tile_map(Scene& scene, entity_id entity);


        vector<uint32_t> counter_{};

        vector<BucketEntry> buckets_{};
        vector<EntityExtent> extents_{};

        vector<CollisionPair> pairs_{};

        friend ::mse::PrivCtx;
    };
}
