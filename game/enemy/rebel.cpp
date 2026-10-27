#include "rebel.hpp"
#include "rebel_anim.hpp"
#include "grenade.hpp"
#include "layer.hpp"
#include "spawn.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        using RebelSM = StateMachine<RebelState>;

        constexpr uvec2 BOX{ 16, 36 };
        constexpr float GRAVITY = 900.0f;

        // in front of the feet, about where the knife reaches
        constexpr uvec2   SENSE_SIZE{ 28, 32 };
        constexpr int32_t SENSE_X = 20;

        // the KnifeSlash frames with the knife out and roughly where it is, from the feet facing left
        constexpr uint32_t KNIFE_FIRST = 9;
        constexpr uint32_t KNIFE_LAST  = 13;
        constexpr ivec2    KNIFE_MIN{ -36, -34 };
        constexpr uvec2    KNIFE_SIZE{ 19, 30 };

        constexpr uint32_t THROW_FRAME = 10; // the hand is empty from here
        constexpr ivec2    HAND{ -8, -36 };
        constexpr uint32_t JUMP_FRAME = 3; // 0-2 are the crouch before it

        constexpr float SNEAK_SPEED  = 30.0f;
        constexpr float RUN_SPEED    = 90.0f;
        constexpr float FLEE_SPEED   = 100.0f; // same as the player's, so he only catches up when it turns back
        constexpr float HOP_VELOCITY = 200.0f; // off a ledge

        // over a wall in front, high enough to clear it by JUMP_MARGIN
        constexpr int32_t JUMP_MAX    = 56;
        constexpr int32_t JUMP_MARGIN = 6;

        // px, feet to feet
        constexpr int32_t SIGHT        = 160;
        constexpr int32_t SIGHT_BEHIND = 80;
        constexpr int32_t SIGHT_Y      = 112;
        constexpr int32_t SAFE         = 200; // running away always turns back past this
        constexpr int32_t FLEE_FAR     = 72;  // what's too close for the least brave
        constexpr int32_t FLEE_NEAR    = 24;  // and for the bravest
        constexpr int32_t THROW_MIN    = 64;
        constexpr int32_t THROW_MAX    = 160;
        constexpr int32_t HEIGHT_DIFF  = 24; // more than this and the knife can't get there
        constexpr int32_t DEADZONE     = 8;  // so it doesn't keep turning around under a player on a platform

        // how much the player has been coming at it or shooting its way lately: +2 a tick he is, -1 a tick he isn't
        constexpr uint8_t PRESSURE_MAX = 60;
        constexpr uint8_t AGGRESSIVE   = 30;

        constexpr uint16_t PANIC_TICKS      = 30;
        constexpr float    CALM_FEAR        = 0.5f; // the fear is scaled by this when the player isn't aggressive
        constexpr float    HIT_AND_RUN      = 0.3f; // added to the fear after a slash
        constexpr uint16_t TURN_BACK_TICKS  = 20;
        constexpr uint16_t CAUTION_TICKS    = 30;
        constexpr float    TURN_BACK_CHANCE = 0.5f;

        constexpr int32_t  GRENADE_CD_MIN = 120;
        constexpr int32_t  GRENADE_CD_MAX = 240;
        constexpr uint32_t MAX_GRENADES   = 3;

        constexpr int32_t WANDER_MIN = 60; // ticks
        constexpr int32_t WANDER_MAX = 150;

        constexpr uint16_t BURN_TICKS  = 48;
        constexpr uint8_t  BLOOD_TICKS = 3;


        // the art sprays to the right, it gets flipped to follow the bullet
        struct BloodSplash final
        {
            anim::Vfx anim;
            ivec2     source; // the pixel in the frame the blood comes out of
            ivec2     chest;  // where on the rebel that goes, from the feet facing left
        };

        constexpr BloodSplash BLOOD[]
        {
            { anim::Vfx::BloodSplash1, { 1, 31 }, { 2, -22 } }, // spray, up and out
            { anim::Vfx::BloodSplash2, { 3, 20 }, { 4, -20 } }, // burst, a bit lower
            { anim::Vfx::BloodSplash3, { 1, 16 }, { 0, -22 } }, // streak straight through
        };


        struct Data final
        {
            entity_id sense;

            float   bravery{}; // how likely to go running away
            int32_t flee_dist{};

            float jump_velocity{};

            uint8_t  pressure{};
            uint16_t grenade_cd{};
            uint16_t wander_ticks{};

            RebelAnim  transition{}; // what Transition and Air play
            RebelState resume{};     // and where they go after

            ivec2 body_offset{}; // the collider's, kept while the knife box replaces it

            bool in_reach{ false }; // set by the sense box, read and cleared the tick after
            bool acted{ false };    // jumped or threw already
        };

        struct Sense final
        {
            entity_id rebel;
        };

        // what the rebel knows about the player this tick
        struct Senses final
        {
            ivec2   to_player; // feet to feet
            int32_t dist;      // on x
            bool    alive;
            bool    behind;
            bool    aggressive; // coming at it or shooting its way
            bool    in_reach;
            bool    ledge;
        };


        Data&   rebel(const RebelSM::Ctx& c) { return c.scene.get<Data>(c.entity); }
        Enemy&  enemy(const RebelSM::Ctx& c) { return c.scene.get<Enemy>(c.entity); }
        Random& rng() { return App::ctx().rng; }

        void play(const RebelSM::Ctx& c, const RebelAnim a) { play_anchored(c.scene, c.entity, a, data(a)); }

        ivec2 map_origin(const Scene& scene)
        {
            return scene.get<Sprite>(scene.bg_entity).bounds(scene.get<Transform>(scene.bg_entity)).min;
        }

        // nothing under the front foot
        bool ledge_ahead(const Scene& scene, const ivec2 at, const int8_t facing)
        {
            const TileMap& tm = App::ctx().tile_map;

            const ivec2 tile = (at + ivec2{ facing * (int32_t)(BOX.x / 2 + 2), 1 } - map_origin(scene)) / TILE_SIZE_i;
            if (tile.x < 0 || tile.y < 0 ||
                tile.x >= (int32_t)tm.size().x || tile.y >= (int32_t)tm.size().y)
                return true;

            return tm[uvec2(tile)] == TileType::Air;
        }

        // how high the wall right in front of the box goes above the feet, 0 or less for no wall
        int32_t wall_ahead(const Scene& scene, const ivec2 at, const int8_t facing)
        {
            const TileMap& tm     = App::ctx().tile_map;
            const ivec2    origin = map_origin(scene);
            const ivec2    local  = at + ivec2{ facing * (int32_t)(BOX.x / 2 + 1), -1 } - origin;

            const ivec2 tile = local / TILE_SIZE_i;
            if (local.x < 0 || local.y < 0 ||
                tile.x >= (int32_t)tm.size().x || tile.y >= (int32_t)tm.size().y)
                return 0;

            int32_t top = tile.y;
            while (top >= 0 && tm[uvec2{ (uint32_t)tile.x, (uint32_t)top }] == TileType::Floor) --top;

            return at.y - (origin.y + (top + 1) * TILE_SIZE_i.y);
        }

        // also counts down the cooldowns and moves the sense box, so every living state calls it first
        Senses sense(const RebelSM::Ctx& c)
        {
            Scene&        scene  = c.scene;
            Data&         d      = rebel(c);
            const int8_t  facing = enemy(c).facing;
            const Player& player = MetalSlug::user_ctx().player;

            const ivec2 me = feet(scene, c.entity);

            Senses s{};
            s.to_player = to_player(scene, c.entity);
            s.dist      = glm::abs(s.to_player.x);
            s.alive     = player.alive(scene);
            s.behind    = s.dist > DEADZONE && (s.to_player.x > 0) != (facing > 0);

            const float vx     = scene.get<Collider>(player.legs()).velocity.x;
            const bool  coming = vx != 0.0f && (vx > 0.0f) == (s.to_player.x < 0);
            const bool  firing = player.shooting(scene) && player.facing(scene) == (s.to_player.x < 0 ? 1 : -1);
            const bool  threat = s.alive && (coming || firing);

            if (threat && d.pressure < PRESSURE_MAX) d.pressure = (uint8_t)(d.pressure + 2);
            else if (!threat && d.pressure > 0) --d.pressure;

            s.aggressive = d.pressure >= AGGRESSIVE;

            s.in_reach = d.in_reach && s.alive;
            d.in_reach = false;

            s.ledge = ledge_ahead(scene, me, facing);

            if (d.grenade_cd > 0) --d.grenade_cd;

            // a tick behind the rebel, a pixel or two
            scene.get<Transform>(d.sense).position = me + ivec2{ facing * SENSE_X, -(int32_t)SENSE_SIZE.y / 2 };

            return s;
        }


        RebelState killed(const RebelSM::Ctx& c) { return enemy(c).burning ? RebelState::Dying : RebelState::Dead; }

        RebelState transition(const RebelSM::Ctx& c, const RebelAnim a, const RebelState next)
        {
            Data& d      = rebel(c);
            d.transition = a;
            d.resume     = next;
            return RebelState::Transition;
        }

        RebelState air(const RebelSM::Ctx& c, const RebelAnim a, const RebelState next)
        {
            transition(c, a, next);
            return RebelState::Air;
        }

        RebelState jump(const RebelSM::Ctx& c, const float velocity, const RebelState next)
        {
            rebel(c).jump_velocity = velocity;
            return air(c, RebelAnim::Jumping, next);
        }

        // the speed to jump over the wall in front, 0 if it's too high
        float jump_over(const RebelSM::Ctx& c)
        {
            const int32_t h = glm::max(wall_ahead(c.scene, feet(c.scene, c.entity), enemy(c).facing), 0);
            if (h > JUMP_MAX) return 0.0f;

            return glm::sqrt(2.0f * GRAVITY * (float)(h + JUMP_MARGIN));
        }

        RebelState panic(const RebelSM::Ctx& c) { return transition(c, RebelAnim::Scared, RebelState::RunAway); }

        // how likely it runs: the cowards more, and more so when the player's aggressive
        float fear(const Data& d, const Senses& s) { return (1.0f - d.bravery) * (s.aggressive ? 1.0f : CALM_FEAR); }

        // only with the player close
        bool panics(const RebelSM::Ctx& c, const Senses& s)
        {
            const Data& d = rebel(c);
            return s.dist < d.flee_dist && rng().real() < fear(d, s);
        }

        // right above it, too close to throw at and out of the knife's reach
        bool overhead(const Senses& s) { return -s.to_player.y > HEIGHT_DIFF && s.dist < THROW_MIN; }

        // walks around for a while, turning first if asked to
        RebelState wander(const RebelSM::Ctx& c, const bool turn)
        {
            rebel(c).wander_ticks = (uint16_t)rng().range(WANDER_MIN, WANDER_MAX);
            return turn ? transition(c, RebelAnim::Turning, RebelState::Wander) : RebelState::Wander;
        }

        bool is_turn(const RebelAnim a)
        {
            return a == RebelAnim::Turning || a == RebelAnim::QuickTurn || a == RebelAnim::ScaredTurn;
        }

        // want if already facing the right way for it (away from the player for RunAway), a turn into it otherwise
        RebelState face_then(const RebelSM::Ctx& c, const Senses& s, const RebelState want)
        {
            const bool away = want == RebelState::RunAway;
            if (s.behind == away) return want;

            RebelAnim turn = RebelAnim::Turning;
            if (away || c.state.current == RebelState::RunAway) turn = RebelAnim::ScaredTurn;
            else if (want == RebelState::Attack) turn = RebelAnim::QuickTurn;

            return transition(c, turn, want);
        }

        // nothing to fight: the cowards run off, the others (and anyone already far) walk away
        RebelState give_up(const RebelSM::Ctx& c, const Senses& s)
        {
            if (s.dist < SAFE && rng().real() < 1.0f - rebel(c).bravery) return face_then(c, s, RebelState::RunAway);
            return wander(c, !s.behind);
        }

        // grenades are for where the knife can't get: the player up on a platform or down a drop.
        // The more cautious ones also throw at a player who keeps his distance
        bool wants_grenade(const RebelSM::Ctx& c, const Senses& s, const bool cautious_roll)
        {
            const Data& d = rebel(c);

            if (d.grenade_cd > 0 || !s.alive || s.dist < THROW_MIN || s.dist > THROW_MAX) return false;
            if (live_grenades(c.scene) >= MAX_GRENADES) return false;
            if (glm::abs(s.to_player.y) > HEIGHT_DIFF) return true;

            return cautious_roll && !s.aggressive && rng().real() > d.bravery;
        }


        RebelAnim random_idle() { return (RebelAnim)((int32_t)RebelAnim::Idle1 + rng().range(0, 2)); }

        void idle_enter(const RebelSM::Ctx& c)
        {
            c.scene.get<Collider>(c.entity).velocity.x = 0.0f;
            play(c, random_idle());
        }

        RebelState on_idle(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            if (enemy(c).shot) return killed(c);

            if (c.scene.get<Sprite>(c.entity).anim_done()) play(c, random_idle());

            const bool spotted = s.alive &&
                    glm::abs(s.to_player.y) < SIGHT_Y &&
                    s.dist < (s.behind ? SIGHT_BEHIND : SIGHT);

            if (!spotted) return RebelState::Idle;
            if (panics(c, s)) return panic(c);

            return face_then(c, s, s.aggressive ? RebelState::Attack : RebelState::Sneak);
        }


        void transition_enter(const RebelSM::Ctx& c)
        {
            c.scene.get<Collider>(c.entity).velocity.x = 0.0f;
            play(c, rebel(c).transition);
        }

        RebelState on_transition(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            Data&        d = rebel(c);
            if (enemy(c).shot) return killed(c);

            if (!c.scene.get<Sprite>(c.entity).anim_done()) return RebelState::Transition;

            if (is_turn(d.transition))
            {
                Enemy& en = enemy(c);
                en.facing = (int8_t)-en.facing;
                return d.resume;
            }

            // after getting scared or stopping it may still have to turn, that's a second transition without leaving this one
            const RebelState next = face_then(c, s, d.resume);
            if (next == RebelState::Transition) play(c, d.transition);

            return next;
        }


        void run_away_enter(const RebelSM::Ctx& c) { play(c, RebelAnim::RunningAway); }

        RebelState on_run_away(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            Data&        d = rebel(c);
            if (enemy(c).shot) return killed(c);

            Collider& col = c.scene.get<Collider>(c.entity);

            // the tile map zeroes the velocity on a wall
            const bool cornered = c.state.ticks > 1 && col.velocity.x == 0.0f;
            col.velocity.x      = (float)enemy(c).facing * FLEE_SPEED;

            if (!col.grounded) return air(c, RebelAnim::Falling, RebelState::RunAway);

            if (cornered)
            {
                if (const float v = jump_over(c);
                    v > 0.0f)
                    return jump(c, v, RebelState::RunAway);
                return face_then(c, s, RebelState::Attack);
            }

            if (s.dist > SAFE) return s.alive ? face_then(c, s, RebelState::Sneak) : wander(c, false);
            if (!s.alive) return RebelState::RunAway;

            // the further it got, the likelier it turns back, less so with the player still after it
            const float far    = (float)glm::min(s.dist, SIGHT) / (float)SIGHT;
            const float chased = s.aggressive ? 0.5f : 1.0f;

            if (c.state.ticks % TURN_BACK_TICKS == 0 &&
                rng().real() < TURN_BACK_CHANCE * far * chased * (0.5f + d.bravery))
            {
                if (wants_grenade(c, s, true)) return face_then(c, s, RebelState::Throw);
                return face_then(c, s, s.aggressive ? RebelState::Attack : RebelState::Sneak);
            }

            return RebelState::RunAway;
        }


        void sneak_enter(const RebelSM::Ctx& c) { play(c, RebelAnim::Sneaking); }

        RebelState on_sneak(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            if (enemy(c).shot) return killed(c);

            Collider& col = c.scene.get<Collider>(c.entity);

            const bool blocked = c.state.ticks > 1 && col.velocity.x == 0.0f;
            col.velocity.x     = (float)enemy(c).facing * SNEAK_SPEED;

            if (!col.grounded) return air(c, RebelAnim::Falling, RebelState::Sneak);
            if (s.in_reach) return RebelState::Slash;

            if (blocked)
            {
                if (const float v = jump_over(c);
                    v > 0.0f)
                    return jump(c, v, RebelState::Sneak);
                return wander(c, true);
            }

            if (!s.alive) return give_up(c, s);
            if (overhead(s)) return wander(c, !s.behind);
            if (c.state.ticks % PANIC_TICKS == 0 && panics(c, s)) return panic(c);
            if (s.aggressive) return face_then(c, s, RebelState::Attack);
            if (wants_grenade(c, s, c.state.ticks % CAUTION_TICKS == 0)) return face_then(c, s, RebelState::Throw);

            return face_then(c, s, RebelState::Sneak);
        }


        void attack_enter(const RebelSM::Ctx& c) { play(c, RebelAnim::Running); }

        RebelState on_attack(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            Data&        d = rebel(c);
            if (enemy(c).shot) return killed(c);

            Collider& col = c.scene.get<Collider>(c.entity);

            const bool blocked = c.state.ticks > 1 && col.velocity.x == 0.0f;
            col.velocity.x     = (float)enemy(c).facing * RUN_SPEED;

            if (!col.grounded) return air(c, RebelAnim::Falling, RebelState::Attack);
            if (!s.alive) return give_up(c, s);
            if (overhead(s)) return wander(c, !s.behind);
            if (s.in_reach) return RebelState::Slash;
            if (s.behind) return face_then(c, s, RebelState::Attack);
            if (c.state.ticks % PANIC_TICKS == 0 && panics(c, s)) return panic(c);

            if (blocked)
            {
                if (const float v = jump_over(c);
                    v > 0.0f)
                    return jump(c, v, RebelState::Attack);

                return wander(c, true);
            }

            if (s.ledge) return jump(c, HOP_VELOCITY, RebelState::Attack);
            if (wants_grenade(c, s, false)) return RebelState::Throw;
            if (d.pressure == 0) return transition(c, RebelAnim::Stopping, RebelState::Sneak);

            return RebelState::Attack;
        }


        void wander_enter(const RebelSM::Ctx& c) { play(c, RebelAnim::Sneaking); }

        // turns at drops and at walls it can't jump, when the time's up it goes back at the player if there's a point
        RebelState on_wander(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            Data&        d = rebel(c);
            if (enemy(c).shot) return killed(c);

            Collider& col = c.scene.get<Collider>(c.entity);

            const bool blocked = c.state.ticks > 1 && col.velocity.x == 0.0f;
            col.velocity.x     = (float)enemy(c).facing * SNEAK_SPEED;

            if (!col.grounded) return air(c, RebelAnim::Falling, RebelState::Wander);
            if (s.in_reach) return RebelState::Slash;

            if (blocked)
            {
                if (const float v = jump_over(c);
                    v > 0.0f)
                    return jump(c, v, RebelState::Wander);
                return transition(c, RebelAnim::Turning, RebelState::Wander);
            }

            if (s.ledge) return transition(c, RebelAnim::Turning, RebelState::Wander);

            if (d.wander_ticks > 0)
            {
                --d.wander_ticks;
                return RebelState::Wander;
            }

            // pacing around until the player's back, and keeps walking off if he's still right above
            if (!s.alive) return wander(c, rng().range(0, 1) == 0);
            if (overhead(s)) return wander(c, false);

            return face_then(c, s, s.aggressive ? RebelState::Attack : RebelState::Sneak);
        }


        void air_enter(const RebelSM::Ctx& c)
        {
            Data& d = rebel(c);
            d.acted = false;

            if (d.transition == RebelAnim::Jumping) c.scene.get<Collider>(c.entity).velocity.x = 0.0f;
            play(c, d.transition);
        }

        RebelState on_air(const RebelSM::Ctx& c)
        {
            sense(c);
            Data& d = rebel(c);
            if (enemy(c).shot) return killed(c);

            Collider& col = c.scene.get<Collider>(c.entity);

            if (d.transition == RebelAnim::Jumping)
            {
                if (!d.acted)
                {
                    if (c.scene.get<Sprite>(c.entity).frame() < JUMP_FRAME) return RebelState::Air;

                    col.velocity.y = -d.jump_velocity;
                    col.grounded   = false; // until the next step says otherwise
                    d.acted        = true;
                }

                // every tick, a wall it's going over zeroes it until it's above it
                col.velocity.x = (float)enemy(c).facing * RUN_SPEED;
            }

            return col.grounded ? d.resume : RebelState::Air;
        }


        // the knife box replaces the body box on the frames the knife is out.
        // Nothing can shoot it then, and the box isn't standing on anything, so no gravity or tiles either
        void knife(const RebelSM::Ctx& c, const bool out)
        {
            Collider&   col = c.scene.get<Collider>(c.entity);
            const Data& d   = rebel(c);

            if (out == (col.layer == (uint32_t)Layer::EnemyKnife)) return;

            if (!out)
            {
                col.offset       = d.body_offset;
                col.size         = BOX;
                col.layer        = (uint32_t)Layer::Enemy;
                col.target_layer = (uint32_t)Layer::PlayerProjectile;
                col.gravity      = GRAVITY;
                col.collide_with_tile_map = true;
                return;
            }

            ivec2 center = KNIFE_MIN + ivec2(KNIFE_SIZE / 2u);
            if (enemy(c).facing > 0) center.x = -center.x;

            col.offset                = d.body_offset + ivec2{ 0, (int32_t)BOX.y / 2 } + center;
            col.size                  = KNIFE_SIZE;
            col.layer                 = (uint32_t)Layer::EnemyKnife;
            col.target_layer          = 0;
            col.gravity               = 0.0f;
            col.velocity              = vec2{ 0.0f };
            col.collide_with_tile_map = false;
        }

        void slash_enter(const RebelSM::Ctx& c)
        {
            Collider& col  = c.scene.get<Collider>(c.entity);
            col.velocity.x = 0.0f;

            play(c, RebelAnim::KnifeSlash);
            rebel(c).body_offset = col.offset;

            MetalSlug::user_ctx().sounds.play(Sfx::Knife);
        }

        RebelState on_slash(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            if (enemy(c).shot) return killed(c);

            const Sprite& spr = c.scene.get<Sprite>(c.entity);
            knife(c, spr.frame() >= KNIFE_FIRST && spr.frame() <= KNIFE_LAST);

            if (!spr.anim_done()) return RebelState::Slash;

            // hit and run, the cowards more often
            if (rng().real() < fear(rebel(c), s) + HIT_AND_RUN) return face_then(c, s, RebelState::RunAway);
            return face_then(c, s, s.aggressive ? RebelState::Attack : RebelState::Sneak);
        }

        void slash_exit(const RebelSM::Ctx& c) { knife(c, false); }


        void throw_enter(const RebelSM::Ctx& c)
        {
            c.scene.get<Collider>(c.entity).velocity.x = 0.0f;
            rebel(c).acted                             = false;
            play(c, RebelAnim::ThrowingGrenade);
        }

        RebelState on_throw(const RebelSM::Ctx& c)
        {
            const Senses s = sense(c);
            Data&        d = rebel(c);
            if (enemy(c).shot) return killed(c);

            if (!d.acted && c.scene.get<Sprite>(c.entity).frame() >= THROW_FRAME)
            {
                const ivec2 me = feet(c.scene, c.entity);
                const ivec2 hand{ enemy(c).facing < 0 ? HAND.x : -HAND.x, HAND.y };

                // creates an entity, no component references across this
                launch_grenade(c.scene, me + hand, me + s.to_player);

                d.grenade_cd = (uint16_t)rng().range(GRENADE_CD_MIN, GRENADE_CD_MAX);
                d.acted      = true;
            }

            if (!c.scene.get<Sprite>(c.entity).anim_done()) return RebelState::Throw;
            return face_then(c, s, s.aggressive ? RebelState::Attack : RebelState::Sneak);
        }


        void die(const RebelSM::Ctx& c)
        {
            stop_colliding(c.scene, c.entity);
            c.scene.destroy_entity(rebel(c).sense);
        }

        void dying_enter(const RebelSM::Ctx& c)
        {
            die(c);
            play(c, RebelAnim::OnFireBig);

            MetalSlug::user_ctx().sounds.rebel_death();
        }

        RebelState on_dying(const RebelSM::Ctx& c)
        {
            return c.state.ticks >= BURN_TICKS ? RebelState::Dead : RebelState::Dying;
        }

        void splash_blood(const RebelSM::Ctx& c)
        {
            const BloodSplash& b  = BLOOD[(size_t)rng().range(0, (int32_t)std::size(BLOOD) - 1)];
            const Enemy&       en = enemy(c);

            // where it comes out of is fixed to the rebel, which way it sprays follows the bullet
            const ivec2   chest = feet(c.scene, c.entity) + ivec2{ en.facing < 0 ? b.chest.x : -b.chest.x, b.chest.y };
            const int8_t  dir   = en.shot_dir != 0 ? en.shot_dir : (int8_t)-en.facing;
            const int32_t layer = c.scene.get<Sprite>(c.entity).layer + 1;

            const entity_id splash = MetalSlug::user_ctx().vfx.create(c.scene, {}, b.anim, BLOOD_TICKS, layer);

            Sprite& spr = c.scene.get<Sprite>(splash);
            spr.flip_x  = dir < 0;

            ivec2 source = b.source;
            if (spr.flip_x) source.x = (int32_t)spr.size().x - 1 - source.x;

            Transform& tr = c.scene.get<Transform>(splash);
            tr.position   += chest - (spr.bounds(tr).min + source);
        }

        void dead_enter(const RebelSM::Ctx& c)
        {
            die(c);

            // burning ones come through Dying, they've screamed already
            if (enemy(c).burning)
            {
                play(c, RebelAnim::DeathFireBig);
                return;
            }

            MetalSlug::user_ctx().sounds.rebel_death();

            // 6 normal deaths and 2 bloody ones, the bloody ones have the blood drawn in already
            const int32_t r = rng().range(0, 7);
            if (r >= 6)
            {
                play(c, (RebelAnim)((int32_t)RebelAnim::BloodyDeath1 + r - 6));
                return;
            }

            play(c, (RebelAnim)((int32_t)RebelAnim::Death1 + r));
            splash_blood(c);
        }

        // blinks out after the animation
        RebelState on_dead(const RebelSM::Ctx& c)
        {
            Sprite& spr = c.scene.get<Sprite>(c.entity);
            if (!spr.anim_done()) return RebelState::Dead;

            // nothing's left of him at the end of that one anyway
            if (spr.anim<RebelAnim>() == RebelAnim::BloodyDeath1)
            {
                c.scene.destroy_entity(c.entity);
                return RebelState::Dead;
            }

            blink_out(c.scene, c.entity);
            return RebelState::Dead;
        }


        // the legs only, the body's box sticks out with the gun and it'd stab at the air in front of him
        void on_sense(Scene& scene, const entity_id self, const entity_id other, uint32_t)
        {
            if (other != MetalSlug::user_ctx().player.legs()) return;
            scene.get<Data>(scene.get<Sense>(self).rebel).in_reach = true;
        }


        using B = RebelSM::Behaviour;
        constexpr RebelSM rebel_sm
        {
            B{ idle_enter, on_idle, nullptr },             // Idle
            B{ transition_enter, on_transition, nullptr }, // Transition
            B{ run_away_enter, on_run_away, nullptr },     // RunAway
            B{ sneak_enter, on_sneak, nullptr },           // Sneak
            B{ attack_enter, on_attack, nullptr },         // Attack
            B{ wander_enter, on_wander, nullptr },         // Wander
            B{ air_enter, on_air, nullptr },               // Air
            B{ slash_enter, on_slash, slash_exit },        // Slash
            B{ throw_enter, on_throw, nullptr },           // Throw
            B{ dying_enter, on_dying, nullptr },           // Dying
            B{ dead_enter, on_dead, nullptr },             // Dead
        };
    }


    void RebelManager::register_machine() const { App::ctx().state_machines.add(rebel_sm); }

    entity_id RebelManager::spawn(Scene& scene, const ivec2 pos) const
    {
        const entity_id e = spawn_standing(scene, pos, RebelAnim::Idle1);

        Data& d      = scene.set<Data>(e);
        d.bravery    = rng().real();
        d.flee_dist  = FLEE_FAR - (int32_t)((float)(FLEE_FAR - FLEE_NEAR) * d.bravery);
        d.grenade_cd = (uint16_t)rng().range(0, GRENADE_CD_MAX); // so they don't all throw on the same tick

        // a third look away from where the player comes from
        scene.set<Enemy>(e).facing = (int8_t)(rng().range(0, 2) == 0 ? 1 : -1);

        const ivec2 box_center = pos - ivec2{ 0, (int32_t)BOX.y / 2 };
        Collider&   col        = scene.set<Collider>(e, box_center - scene.get<Transform>(e).position, ivec2(BOX));

        col.gravity      = GRAVITY;
        col.layer        |= Layer::Enemy;
        col.target_layer |= Layer::PlayerProjectile;
        col.callback     = on_enemy_shot;

        // knife reach, follows the rebel in sense()
        d.sense = scene.create_entity();
        scene.set<Transform>(d.sense);
        scene.set<Sense>(d.sense, e);

        Collider& reach = scene.set<Collider>(d.sense, ivec2{ 0 }, ivec2(SENSE_SIZE));

        reach.gravity               = 0.0f;
        reach.collide_with_tile_map = false;
        reach.target_layer          |= Layer::Player;
        reach.callback              = on_sense;

        rebel_sm.start(scene, e, RebelState::Idle);
        return e;
    }
}
