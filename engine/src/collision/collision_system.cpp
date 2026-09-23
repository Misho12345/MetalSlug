#include "mse/pch.hpp"
#include "collision_system.hpp"

#include "mse/ecs/scene.hpp"
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

    CollisionSystem::CollisionSystem() { objects_.reserve(64); }

    bool CollisionSystem::pp_check(const SpriteData& a, const SpriteData& b)
    {
        const SpriteDataRegistry& reg = App::priv_ctx().sprite_data_registry;

        const aabb bounds_a = a.sr.bounds(a.tr);
        const aabb bounds_b = b.sr.bounds(b.tr);

        const aabb overlap = aabb::overlap(bounds_a, bounds_b);

        if (!overlap) return false;

        const glm::ivec2 size = overlap.size();
        const glm::ivec2 local_a = overlap.min - bounds_a.min;
        const glm::ivec2 local_b = overlap.min - bounds_b.min;

        const FrameMask mask_a = reg.mask(a.sr.info(), a.sr.frame());
        const FrameMask mask_b = reg.mask(b.sr.info(), b.sr.frame());

        for (int y = 0; y < size.y; ++y)
        {
            for (int x = 0; x < size.x; x += sizeof(size_t))
            {
                const glm::ivec2 coords{ x, y };
                const int max = size.x - x;

                const size_t block_a = mask_a.range(coords + local_a, max);
                const size_t block_b = mask_b.range(coords + local_b, max);

                if (block_a & block_b) return true;
            }
        }

        return false;
    }

    void CollisionSystem::step(Scene& scene)
    {
        ComponentData tr_data{ scene.pool<Transform>() };
        ComponentData sr_data{ scene.pool<SpriteRenderer>() };
        ComponentData sc_data{ scene.pool<SpriteCollider>() };

        if (!tr_data || !sc_data) return;

        objects_.clear();

        // get all dynamic objects
        for (size_t i = 0; i < sc_data.size; ++i)
        {
            Transform* tr = sc_data.get_from_idx(i, tr_data);
            if (!tr) continue;

            SpriteRenderer* sr = sc_data.get_from_idx(i, sr_data);
            if (!sr) continue;

            SpriteCollider& col = sc_data.components[i];

            // update rb while at it
            // if (col.gravity) rb.acceleration.y += GRAVITY * 10.0f;
            col.velocity += std::exchange(col.acceleration, { 0.0f, 0.0f }) * FIXED_TIME_STEP;
            col.velocity *= 1.0f - col.drag;

            objects_.emplace_back(*tr, *sr, col);
        }

        for (SpriteData& obj : objects_)
        {
            obj.sc.pos_remainder  += obj.sc.velocity * FIXED_TIME_STEP;

            const glm::vec2 floor = glm::floor(obj.sc.pos_remainder);

            obj.tr.position      += floor;
            obj.sc.pos_remainder -= floor;
        }

        // vector<float> results = all_sweeps();
        //
        // for (size_t i = 0; i < results.size(); ++i)
        // {
        //     SpriteData& obj = objects_[i];
        //
        //     obj.sc.pos_remainder += obj.sc.velocity * results[i] * FIXED_TIME_STEP;
        //
        //     const glm::vec2 floor = glm::floor(obj.sc.pos_remainder);
        //     obj.tr.position       += floor;
        //     obj.sc.pos_remainder -= floor;
        // }
    }

    // vector<float> CollisionSystem::all_sweeps()
    // {
    //     const size_t s = objects_.size();
    //
    //     // smallest sweep result for every dynamic object with other dynamic objects
    //     vector<float> results_dyn;
    //     results_dyn.resize(s, 1.0f);
    //
    //     // indices of the other dynamic object the current sweep is at
    //     vector<size_t> other_dyn_idx;
    //     other_dyn_idx.resize(s, -1_zu);
    //
    //     // indices of invalid sweep results for the dynamic objects results, i.e. they need recalculation
    //     vector<size_t> invalid;
    //     vector<size_t> temp_invalid;
    //
    //     invalid.reserve(s);
    //     temp_invalid.reserve(8);
    //
    //     // mark all indices "invalid" at the beginning
    //     for (size_t i = 0; i < s; ++i) invalid.emplace_back(i);
    //
    //     while (true)
    //     {
    //         for (size_t i = 0; i < invalid.size(); ++i)
    //         {
    //             const size_t  idx1 = invalid[i];
    //             const SpriteData& obj1 = objects_[idx1];
    //
    //             for (size_t j = i; j < invalid.size(); ++j)
    //             {
    //                 const size_t  idx2 = invalid[j];
    //                 const SpriteData& obj2 = objects_[idx2];
    //
    //                 const float result = sweep(
    //                     obj1.sc.bounds(obj1.tr), obj1.sc.velocity,
    //                     obj2.sc.bounds(obj2.tr), obj2.sc.velocity);
    //
    //                 // skip if the result is not smaller than both objects
    //                 if (result >= results_dyn[idx1] &&
    //                     result >= results_dyn[idx2])
    //                     continue;
    //
    //                 if (other_dyn_idx[idx1] != -1_zu) temp_invalid.emplace_back(idx1);
    //                 if (other_dyn_idx[idx2] != -1_zu) temp_invalid.emplace_back(idx2);
    //
    //                 other_dyn_idx[idx1] = idx2;
    //                 other_dyn_idx[idx2] = idx1;
    //
    //                 results_dyn[idx1] = result;
    //                 results_dyn[idx2] = result;
    //             }
    //         }
    //
    //         if (temp_invalid.empty()) break;
    //         swap(temp_invalid, invalid);
    //         temp_invalid.clear();
    //     }
    //
    //     return results_dyn;
    // }


    float CollisionSystem::sweep(
        const aabb bounds_a, const glm::vec2 vel_a,
        const aabb bounds_b, const glm::vec2 vel_b)
    {
        const glm::vec2 vel = (vel_a - vel_b) * FIXED_TIME_STEP;

        const bool vel_x_zero = zero(vel.x);
        const bool vel_y_zero = zero(vel.y);

        // still or same velocity
        if (vel_x_zero && vel_y_zero) return 1.0f;

        aabb box{
            .min = bounds_b.min - bounds_a.max, // same as B.min - half_size - A
            .max = bounds_b.max - bounds_a.min  // same as B.max + half_size - A
        };

        // already inside
        if (box.contains_excl(glm::vec2{ 0.0f })) return 1.0f;

        glm::vec2 enter, exit;

        if (vel_x_zero)
        {
            if (box.min.x > 0.0f || box.max.x < 0.0f) return 1.0f; // miss
            enter.x = -INFINITY;
            exit.x  = INFINITY;
        }
        else
        {
            const float t1 = box.min.x / vel.x;
            const float t2 = box.max.x / vel.x;
            enter.x        = min(t1, t2);
            exit.x         = max(t1, t2);
        }

        if (vel_y_zero)
        {
            if (box.min.y > 0.0f || box.max.y < 0.0f) return 1.0f; // miss
            enter.y = -INFINITY;
            exit.y  = INFINITY;
        }
        else
        {
            const float t1 = box.min.y / vel.y;
            const float t2 = box.max.y / vel.y;
            enter.y        = min(t1, t2);
            exit.y         = max(t1, t2);
        }

        const float t_enter = max(enter.x, enter.y);
        const float t_exit  = min(exit.x, exit.y);

        if (t_enter <= t_exit && t_exit > 0.0f && t_enter < 1.0f) return max(t_enter, 0.0f);
        return 1.0f;
    }
}
