#include "mse/pch.hpp"
#include "collision_system.hpp"

#include "mse/ecs/scene.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    namespace
    {
        constexpr uint32_t size_t_bits = sizeof(size_t) * 8;

        // how far a body can be lifted onto the surface under its center
        constexpr int max_step = TILE_SIZE_i.y / 2;
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

            sc.velocity.y = min(sc.velocity.y + sc.gravity * Target::FRAME_TIME, sc.max_fall_speed);

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
                    if (max(a->starting_grid_id, b->starting_grid_id) == c &&
                        collision_check(scene, a->entity, b->entity))
                        pairs_.emplace_back(a->entity, b->entity);
                }
            }
        }

        for (size_t i = 0; i < entities.size(); ++i)
        {
            SpriteCollider& sc = sc_comps[i];
            Transform&      tr = scene.get<Transform>(entities[i]);

            const ivec2 whole = sc.move;
            tr.position += whole;
            sc.move -= whole;
        }

        for (const CollisionPair& pair : pairs_)
        {
            const SpriteCollider& sc_a = scene.get<SpriteCollider>(pair.a);
            const SpriteCollider& sc_b = scene.get<SpriteCollider>(pair.b);

            if (sc_a.target_layer & sc_b.layer && sc_a.callback) sc_a.callback(pair.b, sc_b.layer);
            if (sc_b.target_layer & sc_a.layer && sc_b.callback) sc_b.callback(pair.a, sc_a.layer);
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

        // move is increased by vel * dt in ::step()
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
        const FrameMask mask_a = reg.mask(sr_a.info(), sr_a.frame(), sr_a.flip_x);
        const FrameMask mask_b = reg.mask(sr_b.info(), sr_b.frame(), sr_b.flip_x);

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
                // the next chunk of 2 is requested, and the bits from 1 which
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

        const TileType* tiles   = tm.data();
        const ivec2     tm_size = tm.size();

        // local to tile map space
        aabb bounds = sc.bounds(tr) - (tm_offset - ivec2(tm_image_size / 2));

        const bool was_grounded = sc.grounded;
        sc.grounded = false;

        const Auto to_tile = [](const int px, const int a)
        {
            // floors (negative px -> negative tile)
            return (px < 0 ? px - TILE_SIZE_i[a] + 1 : px) / TILE_SIZE_i[a];
        };

        const Auto tile_at = [&](const int tx, const int ty)
        {
            // everything outside the map is air
            return tx < 0 || ty < 0 || tx >= tm_size.x || ty >= tm_size.y
                       ? TileType::Air
                       : tiles[tx + ty * tm_size.x];
        };

        // ends the move along the axis after dist px
        const Auto stop = [&](const int a, const int dist)
        {
            sc.move[a] = (float)dist;
            sc.velocity[a] = 0.0f;
        };

        // is the tile a wall (a == 0) / ceiling (a == 1) for something entering it in direction step
        const Auto blocks = [&](const int tx, const int ty, const int a, const int step)
        {
            const TileType tile = tile_at(tx, ty);

            // the slope that goes up in the direction of movement
            const TileType uphill = step > 0 ? TileType::SlopeR : TileType::SlopeL;

            switch (tile)
            {
                case TileType::Air: [[fallthrough]];
                case TileType::Platform: return false;

                // slopes are walls only from their tall side
                case TileType::SlopeL: [[fallthrough]];
                case TileType::SlopeR: return a == 1 || tile != uphill;

                // the floor at the high end of a slope is where the slope leads to, not a wall
                case TileType::Floor: return a == 1 || tile_at(tx - step, ty) != uphill;

                default: assert(!"not implemented"); return false;
            }
        };

        // axis sweep (walls for x, ceilings for y)
        const Auto sweep = [&](const int a)
        {
            const int p    = 1 - a;
            const int dist = (int)sc.move[a];
            const int dir  = dist > 0 ? 1 : 0;
            const int step = dir ? 1 : -1;

            // perpendicular range (max is exclusive)
            const int p_start  = to_tile(bounds.min[p], p);
            const int p_target = to_tile(bounds.max[p] - 1, p);

            // primary axis range
            // from the first tile after the leading edge (the ones it's already in can't be ran into)
            // to the one the leading edge ends up in
            const int lead     = bounds[dir][a] - dir;
            const int a_start  = to_tile(lead, a) + step;
            const int a_target = to_tile(lead + dist, a);

            for (int ta = a_start; ta * step <= a_target * step; ta += step)
            {
                for (int tp = p_start; tp <= p_target; ++tp)
                {
                    if (!blocks(a ? tp : ta, a ? ta : tp, a, step)) continue;

                    // up to the near side of the tile
                    stop(a, (ta + 1 - dir) * TILE_SIZE_i[a] - bounds[dir][a]);
                    return;
                }
            }
        };

        // finds the highest surface under the box and lands on it if it's close enough
        const Auto land = [&](const int dx)
        {
            const int bottom   = bounds.max.y;
            const int center_x = bounds.center().x;
            const int center_t = to_tile(center_x, 0);

            // if grounded, follow the ground down (slopes) instead of flying off and falling back on it
            const int reach = (int)sc.move.y + (was_grounded ? glm::abs(dx) + 2 : 0);

            const int y_start  = to_tile(bottom - max_step, 1);
            const int y_target = to_tile(bottom + reach, 1);
            const int x_start  = to_tile(bounds.min.x, 0);
            const int x_target = to_tile(bounds.max.x - 1, 0);

            int surface = INT_MAX;

            for (int ty = y_start; ty <= y_target; ++ty)
            {
                for (int tx = x_start; tx <= x_target; ++tx)
                {
                    const TileType tile = tile_at(tx, ty);

                    // on which side of the tile the center is (0 if it's above/in it)
                    const int side = (center_t > tx) - (center_t < tx);

                    int  s       = ty * TILE_SIZE_i.y; // surface of this tile
                    bool step_up = false;              // can it be above the bottom (up to max_step)

                    switch (tile)
                    {
                        case TileType::Air: continue;

                        case TileType::Floor: [[fallthrough]];
                        case TileType::Platform:
                            // the tile a slope goes down from doesn't hold the box when its center is on that slope
                            // (otherwise it hangs on the ledge and then drops, instead of walking down)
                            if (side && tile_at(tx + side, ty) == (side > 0 ? TileType::SlopeL : TileType::SlopeR)) continue;

                            // getting off the top of a slope leaves the bottom a pixel or two under the next tile
                            // only when walking, otherwise jumping through a platform would snap onto it
                            step_up = !side && was_grounded;
                            break;

                        case TileType::SlopeL: [[fallthrough]];
                        case TileType::SlopeR:
                        {
                            const bool right = tile == TileType::SlopeR;

                            // center is past the low end
                            if (side == (right ? -1 : 1)) continue;

                            // center is past the high end -> only the high corner counts (the tile's top, like a ledge)
                            if (side) break;

                            // SlopeR (/): low on the left, high on the right;
                            // SlopeL (\): the opposite
                            const int drop = (center_x - tx * TILE_SIZE_i.x) * TILE_SIZE_i.y / TILE_SIZE_i.x;

                            s += right ? TILE_SIZE_i.y - drop : drop;
                            step_up = true; // going uphill the surface is always a bit above the bottom
                            break;
                        }

                        default: assert(!"not implemented");
                    }

                    // it's next to / inside the tile, not above it
                    if (s < bottom - (step_up ? max_step : 0)) continue;

                    surface = min(surface, s); // smaller y = higher
                }
            }

            if (surface > bottom + reach) return; // nothing close enough -> in the air

            stop(1, surface - bottom);
            sc.grounded = true;
        };

        // x axis sweep, then y from the new x
        // (only x is added to the bounds - the y sweep has to start from where the box is)
        sweep(0);

        const int dx = (int)sc.move.x;
        bounds += ivec2{ dx, 0 };

        if ((int)sc.move.y < 0) sweep(1);         // going up
        else if (sc.velocity.y >= 0.0f) land(dx); // falling / standing (not if it just jumped)
    }
}
