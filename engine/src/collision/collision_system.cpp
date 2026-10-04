#include "mse/pch.hpp"
#include "collision_system.hpp"

#include "mse/ecs/scene.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    namespace
    {
        constexpr uint32_t size_t_bits = sizeof(size_t) * 8;

        bool zero(const float x) { return x > -FLT_EPSILON && x < FLT_EPSILON; }
    }

    CollisionSystem::CollisionSystem() : counter_(X_GRID_SIZE + 1)
    {
        buckets_.reserve(128);
        extents_.reserve(128);
        pairs_.reserve(64);
    }


    void CollisionSystem::step(Scene& scene)
    {
        ComponentPool<SpriteCollider>& sc_pool = scene.pool<SpriteCollider>();

        const vector<entity_id>& entities = sc_pool.owners();
        vector<SpriteCollider>& sc_comps = sc_pool.components();


        if (entities.empty()) return;
        for (uint32_t& c : counter_) c = 0;
        buckets_.clear();
        extents_.clear();
        pairs_.clear();

        const ivec2 camera_pos = scene.camera_pos_screen();

        assert(entities.size() == sc_comps.size());

        for (size_t i = 0; i < entities.size(); ++i)
        {
            const entity_id entity = entities[i];
            SpriteCollider& sc = sc_comps[i];

            sc.velocity.y += GRAVITY * 1000.0f * Target::FRAME_TIME;
            sc.move += sc.velocity * Target::FRAME_TIME;

            if (sc.collide_with_tile_map) clamp_move_to_tile_map(scene, entity);

            const Transform&      tr = scene.get<Transform>(entity);
            const SpriteRenderer& sr = scene.get<SpriteRenderer>(entity);

            if (sc.layer == 0 && (sc.target_layer == 0 || !sc.callback)) continue;
            if (sr.parallax_factor != vec2{ 1.0f, 1.0f })
            {
                printf("Collisions with sprites with non-default parallax factor are not possible");
                continue;
            }

            aabb bounds = sr.screen_bounds(tr, camera_pos);
            bounds |= bounds + sc.move;

            if (!(bounds & aabb::screen)) continue;

            // clamp to grid size
            // first is not min.x / size, but like that because operator(int, uint) gives uint
            // which messes makes negative values become super big positive ones
            const uint32_t min_idx = max(bounds.min.x, 0) / X_GRID_CELL_SIZE;
            const uint32_t max_idx = min(bounds.max.x / X_GRID_CELL_SIZE, X_GRID_SIZE - 1);

            // get the cells it'll lie in
            for (uint32_t j = min_idx; j <= max_idx; ++j) ++counter_[j];

            extents_.emplace_back(entity, uvec2{ min_idx, max_idx });
        }


        // turn from counts per bucket to upper boundry for each bucket
        uint32_t curr = 0;
        for (uint32_t& count : counter_) count = curr += count;

        // save the total (that will remain unchanged after the for below)
        // and will be needed for the big looping (below the for below)
        counter_.back() = (&counter_.back())[-1];


        // fill the buckets and move the idx offset from upper to lower boundry for the buckets
        buckets_.resize(counter_.back());
        for (const EntityExtent& extent : extents_)
        {
            for (uint32_t i = extent.range.x; i <= extent.range.y; ++i)
            {
                buckets_[--counter_[i]] = { extent.entity, extent.range.x };
            }
        }

        for (size_t c = 0; c < counter_.size() - 1; ++c)
        {
            const BucketEntry* end = &buckets_[counter_[c + 1]];

            for (BucketEntry* a = &buckets_[counter_[c]]; a < end - 1; ++a)
            {
                for (BucketEntry* b = a + 1; b < end; ++b)
                {
                    // if it's not the first cell they meet in, skip to avoid double checks
                    if (max(a->starting_grid_id, b->starting_grid_id) != c) continue;

                    if (collision_check(scene, a->entity, b->entity)) pairs_.emplace_back(a->entity, b->entity);
                }
            }
        }

        for (size_t i = 0; i < entities.size(); ++i)
        {
            SpriteCollider& sc = sc_comps[i];
            Transform&      tr = scene.get<Transform>(entities[i]);

            const ivec2 whole(sc.move);

            tr.position += whole;
            sc.move -= whole;
        }

        for (const CollisionPair& pair : pairs_)
        {
            const SpriteCollider& sc_a = scene.get<SpriteCollider>(pair.a);
            const SpriteCollider& sc_b = scene.get<SpriteCollider>(pair.b);

            if (sc_a.callback) sc_a.callback(pair.b, sc_b.layer);
            if (sc_b.callback) sc_b.callback(pair.a, sc_a.layer);
        }
    }



    bool CollisionSystem::collision_check(Scene& scene, const entity_id a, const entity_id b)
    {
        Transform& tr_a = scene.get<Transform>(a);
        const SpriteCollider& sc_a = scene.get<SpriteCollider>(a);

        const Transform& tr_b = scene.get<Transform>(b);
        const SpriteCollider& sc_b = scene.get<SpriteCollider>(b);

        if (!(sc_a.target_layer & sc_b.layer) &&
            !(sc_b.target_layer & sc_a.layer))
            return false;

        const float min_size = (float)min(
            sc_a.size.x, sc_a.size.y,
            sc_b.size.x, sc_b.size.y);

        const float min2 = min_size * min_size;

        // pos_remainder is increased by vel * dt in ::step()
        const vec2 move = sc_a.move - sc_b.move;

        const float move_len2 = glm::length2(move);
        const ivec2 pos_a = tr_a.position;

        if (min2 > move_len2)
        {
            const vec2 abs_rel_vel = glm::abs(move);
            bool collided = false;

            if (abs_rel_vel.x > 1.0f || abs_rel_vel.y > 1.0f)
            {
                tr_a.position += ivec2(move);

                collided =
                    sc_a.bounds(tr_a) & sc_b.bounds(tr_b) &&
                    pp_check(scene, a, b);

                tr_a.position = pos_a;
            }

            return collided ||
                    (sc_a.bounds(tr_a) & sc_b.bounds(tr_b) &&
                    pp_check(scene, a, b));
        }

        const float move_len = glm::sqrt(move_len2);
        const float steps_f = move_len / min_size;
        const vec2 step = move / steps_f;

        uint32_t steps_u = (uint32_t)glm::ceil(steps_f);

        bool should_break = false;

        for (vec2 pos = pos_a; !should_break; tr_a.position = ivec2(pos += step))
        {
            if (!steps_u--)
            {
                tr_a.position = pos_a + ivec2(move);
                should_break = true;
            }

            if (sc_a.bounds(tr_a) & sc_b.bounds(tr_b) &&
                pp_check(scene, a, b))
            {
                tr_a.position = pos_a;
                return true;
            }
        }

        tr_a.position = pos_a;
        return false;
    }


    bool CollisionSystem::pp_check(const Scene& scene, const entity_id a, const entity_id b)
    {
        const Transform& tr_a = scene.get<Transform>(a);
        const Transform& tr_b = scene.get<Transform>(b);

        const SpriteRenderer& sr_a = scene.get<SpriteRenderer>(a);
        const SpriteRenderer& sr_b = scene.get<SpriteRenderer>(b);

        const SpriteDataRegistry& reg = App::priv_ctx().sprite_data_registry;

        // metal slug doesn't need this and I'm too lazy to implement it
        if (tr_a.scale != tr_b.scale)
        {
            printf("Pixel perfect checks between differently sized objects is forbidden");
            return false;
        }

        // find intersection (return if none)
        const aabb bounds_a = sr_a.bounds(tr_a);
        const aabb bounds_b = sr_b.bounds(tr_b);

        const aabb overlap = aabb::overlap(bounds_a, bounds_b);

        if (!overlap) return false;

        // get scaled down overlap size and local positions of where the overlap starts for the 2 sprites
        const uvec2 size = overlap.size() / tr_a.scale;

        uvec2 local_a = (overlap.min - tr_a.position) / ivec2(tr_a.scale) + ivec2(sr_a.size()) / 2;
        uvec2 local_b = (overlap.min - tr_b.position) / ivec2(tr_b.scale) + ivec2(sr_b.size()) / 2;

        // The masks (mask_a and mask_b) contain a pointer to a buffer with packed data (see sprite_data_registry.hpp)
        const FrameMask mask_a = reg.mask(sr_a.info(), sr_a.frame());
        const FrameMask mask_b = reg.mask(sr_b.info(), sr_b.frame());

        for (uint32_t y = 0; y < size.y; ++y, ++local_a.y, ++local_b.y)
        {
            // because .range() for the masks requires the coords it's sampled from is byte-aligned
            // in case the first chunks are not aligned i have to read from the beginning of the byte

            // how many bits are to the left from the coords till the start of the byte
            const uint32_t lbits_a = (local_a.x + local_a.y * sr_a.size().x) % 8;
            const uint32_t lbits_b = (local_b.x + local_b.y * sr_b.size().x) % 8;

            // for example
            // for frame with size 21x16 and coords = (15, 15)
            // that lies on the 2nd bit of the 41th byte (0 as MSB)
            // so i have to get the chunk from the start of the 41th byte
            // (lbits will be 2 and rbits will be chunk size - 2)

            if (lbits_a == lbits_b) // the 2 chunks have the same alignment, easier to handle
            {
                for (int32_t x = -(int32_t)lbits_a; x < (int32_t)size.x; x += size_t_bits)
                {
                    const uint32_t max = size.x - x;

                    size_t block_a = mask_a.range(local_a + uvec2{ x, 0 }, max);
                    size_t block_b = mask_b.range(local_b + uvec2{ x, 0 }, max);
                    const size_t mask = x < 0 ? ~((1_zu << lbits_a) - 1_zu) : ~0_zu;

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
                uvec2 local1, local2;
                uint32_t lbits1, lbits2;

                int32_t diff = (int32_t)lbits_b - (int32_t)lbits_a;

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

                uint32_t rev_diff = size_t_bits - diff;


                // (imagine 8 bit chunk size)
                // 1: (- - M L  K J I H) <(G F E D  C B A|-)>
                // 2: (Z Y X W  V U T S) <(R Q P O  N|- - -)>
                //
                // STEP 0:
                // Get the first chunks
                // 1: G F E D  C B A -
                // 2: R Q P O  N - - -
                //
                // STEP 1:
                // 2 is shifted by diff (2 in this case) to align
                // 1: G F E D  C B A -
                // 2: 0 0 R Q  P O N -
                //
                // STEP 2:
                // the next chunk of 2 is requested, and the bytes from 1 which
                // weren't checked because they weren't in 2 have to be checked now
                //
                // 1: [G F] E D  C B A -
                // 2: Z Y X W  V U[T S]
                //
                // STEP 3:
                // 1 is shifted by rev_diff (6 in this case)
                //
                // 1: 0 0 0 0  0 0[G F]
                // 2: Z Y X W  V U[T S]
                //
                // STEP 4:
                // new chunk for 1 is requested
                // 1: [- - M L  K J I H]
                // 2: [Z Y X W  V U]T S
                //
                // STEP 5:
                // 2 is shifted to align
                //
                // 1: [- - M L  K J I H]
                // 2:  0 0[Z Y  X W V U]
                //
                // repeat ...


                // STEP 0
                size_t block1;
                size_t block2 = mask2->range(
                    local2 - uvec2{ lbits2, 0 },
                    size.x + lbits2);

                // start from the beginning of the byte and advance by the chunk size
                for (int32_t x = -(int32_t)lbits1; x < (int32_t)size.x; x += size_t_bits)
                {
                    const uint32_t max = size.x - x;

                    // STEP 0, 4, ...
                    block1 = mask1->range(local1 + uvec2{ x, 0 }, max);

                    if (x < 0)
                    {
                        block1 &= ~((1_zu << lbits1) - 1);
                        block2 &= ~((1_zu << lbits2) - 1);
                    }

                    // STEP 1, 5, ...
                    if (block1 & (block2 >> diff)) return true;

                    if (max < size_t_bits) break; // last => no trailing bits for 2

                    // STEP 2, ...
                    block2 = mask2->range(local2 + uvec2{ x - diff + size_t_bits, 0 }, max);

                    // STEP 3, ...
                    if ((block1 >> rev_diff) & block2) return true;
                }
            }
        }

        return false;
    }

    void CollisionSystem::clamp_move_to_tile_map(Scene& scene, const entity_id entity)
    {
        const Transform& tr = scene.get<Transform>(entity);
        SpriteCollider&  sc = scene.get<SpriteCollider>(entity);

        const TileMap&  tm            = App::priv_ctx().tile_map;
        const ivec2     tm_offset     = scene.get<Transform>(scene.bg_entity).position;
        const uvec2     tm_image_size = scene.get<SpriteRenderer>(scene.bg_entity).size();
        const TileType* tiles         = tm.data();

        const ivec2 tm_size    = tm.size();
        const ivec2 tm_size_px = tm_size * TILE_SIZE_i;

        // local to tile map space
        aabb bounds = sc.bounds(tr) - (tm_offset - ivec2(tm_image_size / 2));

        if (glm::abs(sc.move.x) < 1.0f && glm::abs(sc.move.y) < 1.0f) return;
        if (!(bounds & aabb{ {}, tm_size_px })) return;

        // tile coordinate helpers
        const lambda to_tile_min = [&](const int px, const int a)
        {
            return glm::clamp(px / TILE_SIZE_i[a], 0, tm_size[a] - 1);
        };

        const lambda to_tile_max = [&](const int px, const int a)
        {
            return glm::clamp((px > 0 ? px - 1 : 0) / TILE_SIZE_i[a], 0, tm_size[a] - 1);
        };

        // slope surface height at x
        const lambda get_slope_y = [&](const int tx, const int ty, const TileType type, const float x_center) -> float
        {
            const float tile_left = (float)(tx * TILE_SIZE_i.x);
            const float r         = glm::clamp((x_center - tile_left) / (float)TILE_SIZE_i.x, 0.0f, 1.0f);

            // uphill right (\) vs uphill left (/)
            const float local_y = (type == TileType::SlopeR)
                                      ? (1.0f - r) * (float)TILE_SIZE_i.y
                                      : r * (float)TILE_SIZE_i.y;

            return (float)(ty * TILE_SIZE_i.y) + local_y;
        };

        // axis sweep
        const lambda sweep = [&](const int a)
        {
            if (sc.move[a] == 0.0f) return;

            const int p    = 1 - a;
            const int dir  = sc.move[a] > 0.0f ? 1 : 0;
            const int step = dir ? 1 : -1;

            // perpendicular range
            const int p_start  = to_tile_min(bounds.min[p], p);
            const int p_target = to_tile_max(bounds.max[p], p);

            // primary axis range (gets leading edge based on direction)
            const int lead     = bounds[dir][a];
            const int a_start  = dir ? to_tile_max(lead, a) : to_tile_min(lead, a);
            const int a_target = dir ? to_tile_max(lead + (int)sc.move[a], a) : to_tile_min(lead + (int)sc.move[a], a);

            for (int ta = a_start; step > 0 ? ta <= a_target : ta >= a_target; ta += step)
            {
                for (int tp = p_start; tp <= p_target; ++tp)
                {
                    const int      tx   = a == 0 ? ta : tp;
                    const int      ty   = a == 0 ? tp : ta;
                    const TileType tile = tiles[tx + ty * tm_size.x];

                    if (a == 0) // x axis
                    {
                        const float x_center = (float)(bounds.min.x + bounds.max.x) * 0.5f + sc.move.x;

                        switch (tile)
                        {
                            case TileType::Air: [[fallthrough]];
                            case TileType::Platform: continue;

                            case TileType::SlopeL:
                            {
                                // SlopeL: high on left, low on right.
                                // Non-sloping side is on the right (blocks moving right into it).
                                // Sloping surface allows walking/climbing.
                                if (dir) // moving right -> walking along slope
                                {
                                    const float target_y = get_slope_y(tx, ty, tile, x_center);
                                    sc.move.y            = target_y - (float)bounds.max.y;
                                    continue;
                                }

                                // moving left into the flat right wall of SlopeL -> block
                                sc.move.x = min(0.0f, (float)((tx + 1) * TILE_SIZE_i.x) - bounds.min.x);
                                return;
                            }

                            case TileType::SlopeR:
                            {
                                // SlopeR: low on left, high on right.
                                // Non-sloping side is on the left (blocks moving left into it).
                                // Sloping surface allows walking/climbing.
                                if (!dir) // moving left -> walking along slope
                                {
                                    const float target_y = get_slope_y(tx, ty, tile, x_center);
                                    sc.move.y            = target_y - (float)bounds.max.y;
                                    continue;
                                }

                                // moving right into the flat left wall of SlopeR -> block
                                sc.move.x = max(0.0f, (float)(tx * TILE_SIZE_i.x) - bounds.max.x);
                                return;
                            }

                            case TileType::Floor:
                                sc.move.x = dir
                                  ? max(0.0f, (float)(ta * TILE_SIZE_i.x) - bounds.max.x)
                                  : min(0.0f, (float)((ta + 1) * TILE_SIZE_i.x) - bounds.min.x);
                                return;

                            default: assert(!"not implemented");
                        }
                    }
                    else // y axis
                    {
                        const float x_center = (float)(bounds.min.x + bounds.max.x) * 0.5f;

                        switch (tile)
                        {
                            case TileType::Air: continue;

                            case TileType::SlopeL: [[fallthrough]];
                            case TileType::SlopeR:
                            {
                                if (!dir) // moving up into slope from underneath -> block from bottom
                                {
                                    sc.move.y = min(0.0f, (float)((ty + 1) * TILE_SIZE_i.y) - bounds.min.y);
                                    return;
                                }

                                // falling down onto slope surface
                                const float slope_y = get_slope_y(tx, ty, tile, x_center);
                                if ((float)bounds.max.y + sc.move.y >= slope_y)
                                {
                                    sc.move.y = max(0.0f, slope_y - (float)bounds.max.y);
                                    return;
                                }

                                continue;
                            }

                            case TileType::Platform: if (!dir) continue; // ignore up
                                [[fallthrough]];

                            case TileType::Floor:
                                sc.move.y = dir
                                    ? max(0.0f, (float)(ta * TILE_SIZE_i.y) - bounds.max.y)
                                    : min(0.0f, (float)((ta + 1) * TILE_SIZE_i.y) - bounds.min.y);
                                return;

                            default: assert(!"not implemented");
                        }
                    }
                }
            }
        };

        // x axis sweep (handles horizontal movement and slope elevation changes)
        sweep(0);
        bounds += sc.move;

        // y axis sweep (handles vertical falls, jumps, and bottom slope collisions)
        sweep(1);
    }
}
