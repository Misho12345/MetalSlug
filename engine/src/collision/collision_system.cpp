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

        // metal slug doesn't need this and I'm too lazy to implement it
        if (a.tr.scale != b.tr.scale)
        {
            printf("Pixel perfect checks between differently sized objects is forbidden");
            return false;
        }

        // find intersection (return if none)
        const aabb bounds_a = a.sr.bounds(a.tr);
        const aabb bounds_b = b.sr.bounds(b.tr);

        const aabb overlap = aabb::overlap(bounds_a, bounds_b);

        if (!overlap) return false;

        // get scaled down overlap size and local positions of where the overlap starts for the 2 sprites
        const glm::ivec2 size = overlap.size() / a.tr.scale;

        glm::ivec2 local_a = (overlap.min - a.tr.position) / a.tr.scale + a.sr.size() / 2;
        glm::ivec2 local_b = (overlap.min - b.tr.position) / b.tr.scale + b.sr.size() / 2;

        // The masks (mask_a and mask_b) contain a pointer to a buffer with packed data (see sprite_data_registry.hpp)
        const FrameMask mask_a = reg.mask(a.sr.info(), a.sr.frame());
        const FrameMask mask_b = reg.mask(b.sr.info(), b.sr.frame());

        for (int y = 0; y < size.y; ++y, ++local_a.y, ++local_b.y)
        {
            // because .range() for the masks requires the coords it's sampled from is byte-aligned
            // in case the first chunks are not aligned i have to read from the beginning of the byte

            // how many bits are to the left from the coords till the start of the byte
            const int lbits_a = (local_a.x + local_a.y * a.sr.size().x) % 8;
            const int lbits_b = (local_b.x + local_b.y * b.sr.size().x) % 8;

            // for example
            // for frame with size 21x16 and coords = (15, 15)
            // that lies on the 2nd bit of the 41th byte (0 as MSB)
            // so i have to get the chunk from the start of the 41th byte
            // (lbits will be 2 and rbits will be chunk size - 2)

            if (lbits_a == lbits_b) // the 2 chunks have the same alignment, easier to handle
            {
                for (int x = -lbits_a; x < size.x; x += sizeof(size_t) * 8)
                {
                    const int max = size.x - x;

                    size_t block_a = mask_a.range(local_a + glm::ivec2{ x, 0 }, max);
                    size_t block_b = mask_b.range(local_b + glm::ivec2{ x, 0 }, max);
                    const size_t mask = x < 0 ? ~((1_zu << lbits_a) - 1) : ~0_zu;

                    if (block_a & block_b & mask) return true;
                }
            }
            else
            {
                // Get it so that 1 is the mask with the smaller offset and 2 is the one with the bigger
                // for example if these are where the coords fall in the mask buffers
                // A: - - - -   -|0 1 0 ...; lbits = 5; rbits = 59; => 2
                // B: - - -|1   1 1 0 0 ...; lbits = 3; rbits = 61; => 1

                const FrameMask *mask1, *mask2;
                glm::ivec2 local1, local2;
                int lbits1, lbits2;

                int diff = lbits_b - lbits_a;

                if (diff > 0)
                {
                    mask1 = &mask_a;  mask2 = &mask_b;
                    local1 = local_a; local2 = local_b;
                    lbits1 = lbits_a; lbits2 = lbits_b;
                }
                else
                {
                    mask1 = &mask_b;  mask2 = &mask_a;
                    local1 = local_b; local2 = local_a;
                    lbits1 = lbits_b; lbits2 = lbits_a;

                    diff = -diff;
                }

                int rev_diff = sizeof(size_t) * 8 - diff;


                // (imagine 8 bit chunk size)
                // 1: <(-|A B C  D E F G)>  (H I J K  L M - -)
                // 2: <(- - -|N  O P Q R)>  (S T U V  W X Y Z)
                //
                // STEP 0:
                // Get the first chunks
                // 1: - A B C  D E F G
                // 2: - - - N  O P Q R
                //
                // STEP 1:
                // 2 is shifted by diff (2 in this case) to align
                // 1: - A B C  D E F G
                // 2: - N O P  Q R 0 0
                //
                // STEP 2:
                // the next chunk of 2 is requested, and the bytes from 1 which
                // weren't checked because they weren't in 2 have to be checked now
                //
                // 1:  - A B C  D E[F G]
                // 2: [S T]U V  W X Y Z
                //
                // STEP 3:
                // 1 is shifted by rev_diff (6 in this case)
                //
                // 1: [F G]0 0  0 0 0 0
                // 2: [S T]U V  W X Y Z
                //
                // STEP 4:
                // new chunk for 1 is requested
                // 1: [H I J K  L M - -]
                // 2:  S T[U V  W X Y Z]
                //
                // STEP 5:
                // 2 is shifted to align
                //
                // 1: [H I J K  L M - -]
                // 2: [U V  W X Y Z]0 0
                //
                // repeat ...


                // STEP 0
                size_t block1;
                size_t block2 = mask2->range(
                    local2 - glm::ivec2{ lbits2, 0 },
                    min(static_cast<int>(sizeof(size_t)) * 8, size.x) - lbits1);

                // start from the beginning of the byte and advance by the chunk size
                for (int x = -lbits1; x < size.x; x += sizeof(size_t) * 8)
                {
                    const int max = size.x - x;

                    // STEP 0, 4, ...
                    block1 = mask1->range(local1 + glm::ivec2{ x, 0 }, max);

                    if (x < 0)
                    {
                        block1 &= ~((1_zu << lbits1) - 1);
                        block2 &= ~((1_zu << lbits2) - 1);
                    }

                    // STEP 1, 5, ...
                    if (block1 & (block2 << diff)) return true;

                    if (max < sizeof(size_t) * 8) break; // last => no trailing bits for 2

                    // STEP 2, ...
                    block2 = mask2->range(local2 + glm::ivec2{ x - diff + sizeof(size_t) * 8, 0 }, max);

                    // STEP 3, ...
                    if ((block1 << rev_diff) & block2) return true;
                }
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

            objects_.emplace_back(*tr, *sr, col);
        }

        for (SpriteData& obj : objects_)
        {
            obj.sc.pos_remainder  += obj.sc.velocity * Target::FRAME_TIME;

            const glm::vec2 floor = glm::floor(obj.sc.pos_remainder);

            obj.tr.position      += floor;
            obj.sc.pos_remainder -= floor;
        }


        // ---------------
        const size_t s = sc_data.components.size();
        for (size_t i = 0; i < s - 1; ++i)
        {
            SpriteCollider& sc1 = sc_data.components[i];
            Transform* tr1 = sc_data.get_from_idx(i, tr_data);
            SpriteRenderer* sr1 = sc_data.get_from_idx(i, sr_data);

            if (!tr1 || !sr1) continue;

            entity_id id1 = sc_data.owners[i];

            for (size_t j = i + 1; j < s; ++j)
            {
                SpriteCollider& sc2 = sc_data.components[j];
                Transform* tr2 = sc_data.get_from_idx(j, tr_data);
                SpriteRenderer* sr2 = sc_data.get_from_idx(j, sr_data);

                if (!tr2 || !sr2) continue;

                entity_id id2 = sc_data.owners[j];

                SpriteCollider::callback_t callbacks[2]{};
                uint32_t masks[2]{};

                if (sc1.callback && (masks[0] = sc1.target_mask & sc2.mask)) callbacks[0] = sc1.callback;
                if (sc2.callback && (masks[1] = sc2.target_mask & sc1.mask)) callbacks[1] = sc2.callback;

                if (!callbacks[0] && !callbacks[1]) continue;
                if (!(sc1.bounds(*tr1) & sc2.bounds(*tr2))) continue;

                SpriteData d1{ *tr1, *sr1, sc1 };
                SpriteData d2{ *tr2, *sr2, sc2 };

                if (!pp_check(d1, d2)) continue;

                if (callbacks[0]) callbacks[0](id2, masks[0]);
                if (callbacks[1]) callbacks[1](id1, masks[1]);
            }
        }
    }


    float CollisionSystem::sweep(
        const aabb bounds_a, const glm::vec2 vel_a,
        const aabb bounds_b, const glm::vec2 vel_b)
    {
        const glm::vec2 vel = (vel_a - vel_b) * Target::FRAME_TIME;

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
