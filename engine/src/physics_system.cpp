#include "mse/pch.hpp"
#include "physics_system.hpp"

#include "mse/scene.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    namespace
    {
        bool zero(const float x) { return x > -FLT_EPSILON && x < FLT_EPSILON; }

        template <typename C>
        struct ComponentData final
        {
            explicit ComponentData(ComponentPool<C>& pool)
                : components{ pool.components() },
                  owners{ pool.owners() },
                  lookup{ pool.lookup() },
                  size{ components.size() } { assert(owners.size() == components.size()); }

            vector<C>&               components;
            const vector<entity_id>& owners;
            const vector<int32_t>&   lookup;

            size_t size;

            template <typename O>
            const O* get_from_idx(const size_t idx, const ComponentData<O>& other_data) const
            {
                const entity_id entity = owners[idx];
                if (other_data.lookup.size() <= entity.idx()) return nullptr;

                const int32_t other_idx = other_data.lookup[entity.idx()];
                return other_idx < 0 ? nullptr : &other_data.components[other_idx];
            }

            template <typename O>
            O* get_from_idx(const size_t idx, ComponentData<O>& other_data) const
            {
                return const_cast<O*>(get_from_idx(idx, std::as_const(other_data)));
            }

            operator bool() const { return size; }
        };
    }

    PhysicsSystem::PhysicsSystem()
    {
        dynamic_objects.reserve(64);
        static_objects.reserve(64);
    }

    void PhysicsSystem::step(Scene& scene, [[maybe_unused]] const bool draw_colliders)
    {
        ComponentData rb_data{ scene.pool<Rigidbody>() };
        ComponentData transform_data{ scene.pool<Transform>() };
        ComponentData box_coll_data{ scene.pool<BoxCollider>() };

        if (!transform_data || !rb_data || !box_coll_data) return;

        dynamic_objects.clear();
        static_objects.clear();

        // get all dynamic objects
        for (size_t i = 0; i < rb_data.size; ++i)
        {
            Transform* tr = rb_data.get_from_idx(i, transform_data); if (!tr) continue;
            Rigidbody& rb = rb_data.components[i];
            BoxCollider* bc = rb_data.get_from_idx(i, box_coll_data); if (!bc) continue;

            // update rb while at it
            if (rb.gravity) rb.acceleration.y += GRAVITY * 10.0f;
            rb.velocity += std::exchange(rb.acceleration, { 0.0f, 0.0f }) * FIXED_TIME_STEP;
            rb.velocity *= 1.0f - rb.drag;

            dynamic_objects.emplace_back(*tr, rb, *bc);
        }

        // get all static objects
        for (size_t i = 0; i < box_coll_data.size; ++i)
        {
            Transform* tr = box_coll_data.get_from_idx(i, transform_data); if (!tr) continue;
            BoxCollider& bc = box_coll_data.components[i];
            const Rigidbody* rb = box_coll_data.get_from_idx(i, rb_data); if (rb) continue;

            static_objects.emplace_back(*tr, bc);
        }

        #ifndef NDEBUG
        if (draw_colliders) draw_box_colliders(dynamic_objects, static_objects);
        #endif

        vector<SweepResult> results = all_sweeps();

        for (size_t i = 0; i < results.size(); ++i)
        {
            DynamicObject& dyn = dynamic_objects[i];
            const SweepResult& res = results[i];

            dyn.tr.position += dyn.rb.velocity * res.time * FIXED_TIME_STEP;

            // if stopped, stop the velocity depending on the normal
            if (res.time < 1.0f)
            {
                if (res.normal.x != 0.0f) dyn.rb.velocity.x = 0.0f;
                if (res.normal.y != 0.0f) dyn.rb.velocity.y = 0.0f;

                dyn.tr.position += dyn.rb.velocity * (1.0f - res.time) * FIXED_TIME_STEP;
            }
        }
    }


    #ifndef NDEBUG
    void PhysicsSystem::draw_box_colliders(
        const vector<DynamicObject>& dyn_objs,
        const vector<StaticObject>& stat_objs)
    {
        static constexpr glm::u8vec4 dyn_color{ 255, 0, 0, 255 };
        static constexpr glm::u8vec4 stat_color{ 0, 255, 0, 255 };

        DebugUI& ui = App::priv_ctx().debug_ui;

        for (const DynamicObject& obj : dyn_objs) ui.draw_box(obj.bc.bounds(obj.tr), dyn_color);
        for (const StaticObject& obj : stat_objs) ui.draw_box(obj.bc.bounds(obj.tr), stat_color);
    }
    #endif

    vector<PhysicsSystem::SweepResult> PhysicsSystem::all_sweeps()
    {
        const size_t s = dynamic_objects.size();

        // smallest sweep result for every dynamic object with static objects
        vector<SweepResult> results_stat;
        results_stat.resize(s);

        // smallest sweep result for every dynamic object with other dynamic objects
        vector<SweepResult> results_dyn;
        results_dyn.resize(s);

        // indices of the other dynamic object the current sweep is at
        vector<size_t> other_dyn_idx;
        other_dyn_idx.resize(s, -1_zu);

        // indices of invalid sweep results for the dynamic objects results, i.e. they need recalculation
        vector<size_t> invalid;
        vector<size_t> temp_invalid;

        invalid.reserve(s);
        temp_invalid.reserve(8);

        // get all results from sweeping with static objects
        for (size_t i = 0; i < s; ++i)
        {
            const DynamicObject& dyn = dynamic_objects[i];

            for (const StaticObject& stat : static_objects)
            {
                const SweepResult result = sweep(
                    dyn.bc.bounds(dyn.tr), dyn.rb.velocity,
                    stat.bc.bounds(stat.tr), { 0.0f, 0.0f });

                if (result < results_stat[i]) results_stat[i] = result;
            }
        }

        // mark all indices "invalid" at the beginning
        for (size_t i = 0; i < s; ++i) invalid.emplace_back(i);

        while (true)
        {
            for (size_t i = 0; i < invalid.size(); ++i)
            {
                const size_t idx1 = invalid[i];
                const DynamicObject& dyn1 = dynamic_objects[idx1];

                for (size_t j = i; j < invalid.size(); ++j)
                {
                    const size_t idx2 = invalid[j];
                    const DynamicObject& dyn2 = dynamic_objects[idx2];

                    const SweepResult result = sweep(
                        dyn1.bc.bounds(dyn1.tr), dyn1.rb.velocity,
                        dyn2.bc.bounds(dyn2.tr), dyn2.rb.velocity);

                    // skip if the result is not smaller than both objects
                    if (result >= results_dyn[idx1] &&
                        result >= results_dyn[idx2])
                        continue;

                    if (other_dyn_idx[idx1] != -1_zu) temp_invalid.emplace_back(idx1);
                    if (other_dyn_idx[idx2] != -1_zu) temp_invalid.emplace_back(idx2);

                    other_dyn_idx[idx1] = idx2;
                    other_dyn_idx[idx2] = idx1;

                    results_dyn[idx1] = result;
                    results_dyn[idx2] = result;
                }
            }

            if (temp_invalid.empty()) break;
            swap(temp_invalid, invalid);
            temp_invalid.clear();
        }

        for (size_t i = 0; i < s; ++i)
        {
            results_stat[i] = min(results_stat[i], results_dyn[i]);
        }

        return results_stat;
    }


    PhysicsSystem::SweepResult PhysicsSystem::sweep(
        const aabb bounds_a, const glm::vec2 vel_a,
        const aabb bounds_b, const glm::vec2 vel_b)
    {
        const glm::vec2 vel = (vel_a - vel_b) * FIXED_TIME_STEP;

        const bool vel_x_zero = zero(vel.x);
        const bool vel_y_zero = zero(vel.y);

        // still or same velocity
        if (vel_x_zero && vel_y_zero) return {};

        aabb box {
            .min = bounds_b.min - bounds_a.max, // same as B.min - half_size - A
            .max = bounds_b.max - bounds_a.min  // same as B.max + half_size - A
        };

        // already inside
        if (box.contains_excl(glm::vec2{ 0.0f }))
        {
            glm::vec2 normal{ 0.0f, 0.0f };
            if (abs(vel.x) > abs(vel.y)) normal.x = vel.x > 0.0f ? -1.0f : 1.0f;
            else normal.y = vel.y > 0.0f ? -1.0f : 1.0f;

            return { 1.0f, normal };
        }

        glm::vec2 enter, exit;

        if (vel_x_zero)
        {
            if (box.min.x > 0.0f || box.max.x < 0.0f) return {}; // miss
            enter.x = -INFINITY;
            exit.x = INFINITY;
        }
        else
        {
            const float t1 = box.min.x / vel.x;
            const float t2 = box.max.x / vel.x;
            enter.x = min(t1, t2);
            exit.x  = max(t1, t2);
        }

        if (vel_y_zero)
        {
            if (box.min.y > 0.0f || box.max.y < 0.0f) return {}; // miss
            enter.y = -INFINITY;
            exit.y = INFINITY;
        }
        else
        {
            const float t1 = box.min.y / vel.y;
            const float t2 = box.max.y / vel.y;
            enter.y = min(t1, t2);
            exit.y  = max(t1, t2);
        }

        const float t_enter = max(enter.x, enter.y);
        const float t_exit = min(exit.x, exit.y);

        if (t_enter <= t_exit && t_exit > 0.0f && t_enter < 1.0f)
        {
            glm::vec2 normal{ 0.0f, 0.0f };
            if (enter.x > enter.y) normal.x = vel.x > 0.0f ? -1.0f : 1.0f;
            else normal.y = vel.y > 0.0f ? -1.0f : 1.0f;

            return { max(t_enter, 0.0f), normal };
        }

        return {};
    }
}
