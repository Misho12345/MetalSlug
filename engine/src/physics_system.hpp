#pragma once

#include "mse/app.hpp"

namespace mse
{
    class Scene;
    struct PrivCtx;

    using fixed_update_t = void(App::*)();

    /**
     * @brief Physics system
     * @details This system is responsible for updating the physics components
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class PhysicsSystem final
    {
    public:
        static constexpr uint32_t MAX_STEPS = 10;
        static constexpr float FIXED_TIME_STEP = 1.0f / 60.0f;

        static constexpr float GRAVITY = 9.81f;

        PhysicsSystem(const PhysicsSystem&)            = delete;
        PhysicsSystem(PhysicsSystem&&)                 = delete;
        PhysicsSystem& operator=(const PhysicsSystem&) = delete;
        PhysicsSystem& operator=(PhysicsSystem&&)      = delete;

        void step(Scene& scene, bool draw_colliders);

    private:
        struct DynamicObject final
        {
            Transform& tr;
            Rigidbody& rb;
            BoxCollider& bc;
        };

        struct StaticObject final
        {
            Transform& tr;
            BoxCollider& bc;
        };

        struct SweepResult final
        {
            float time = 1.0f;
            glm::vec2 normal{ 0.0f, 0.0f };

            std::partial_ordering operator<=>(const SweepResult& other) const
            {
                return time <=> other.time;
            }
        };

        PhysicsSystem();

        #ifndef NDEBUG
        static void draw_box_colliders(
            const vector<DynamicObject>& dyn_objs,
            const vector<StaticObject>& stat_objs);
        #endif

        vector<SweepResult> all_sweeps();

        // returns the time of collision (max 1.0f)
        static SweepResult sweep(
            aabb bounds_a, glm::vec2 vel_a,
            aabb bounds_b, glm::vec2 vel_b);

        vector<DynamicObject> dynamic_objects;
        vector<StaticObject> static_objects;

        float time_{ 0.0f };

        friend ::mse::PrivCtx;
    };
}