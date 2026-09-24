#pragma once

#include "mse/app.hpp"

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

        CollisionSystem(const CollisionSystem&)            = delete;
        CollisionSystem(CollisionSystem&&)                 = delete;
        CollisionSystem& operator=(const CollisionSystem&) = delete;
        CollisionSystem& operator=(CollisionSystem&&)      = delete;

        void step(Scene& scene);

    private:
        struct SpriteData final
        {
            Transform& tr;
            SpriteRenderer& sr;
            SpriteCollider& sc;
        };

        CollisionSystem();

        // performs a pixel perfect check using
        static bool pp_check(const SpriteData& a, const SpriteData& b);

        // returns the time of collision (max 1.0f)
        static float sweep(
            aabb bounds_a, glm::vec2 vel_a,
            aabb bounds_b, glm::vec2 vel_b);

        vector<SpriteData> objects_;
        float time_{ 0.0f };

        friend ::mse::PrivCtx;
    };
}