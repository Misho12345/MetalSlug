#include "boss.hpp"
#include "layer.hpp"
#include "projectile.hpp"
#include "metal_slug.hpp"
#include "enemy/enemy.hpp"
#include "enemy/explosive.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        using CannonAnim = anim::boss::Cannon;
        using FxAnim     = anim::boss::CannonFx;
        using BossSM     = StateMachine<BossState>;

        // rips match pixel for pixel
        constexpr ivec2 SHIP_FROM_FRONT{ 448, -64 };

        // the open and fire frames' top left at the low height, from the ship's. Measured off a longplay like
        // the rest of the numbers here
        constexpr ivec2   LOW_FROM_SHIP{ 256, 161 };
        constexpr int32_t HIGH_RISE = 30;

        // the other strips' top left from the open strip's, they're cut to different heights
        constexpr ivec2 MOVE_OFF{ 0, -17 };
        constexpr ivec2 DESTROYED_OFF{ 10, 2 };

        // from the open strip's top left. Up high it still goes down to where the bottom is when it's low, the
        // bullets from a standing player go under it otherwise
        constexpr ivec2 BOX_MIN{ 2, 8 };
        constexpr uvec2 BOX_SIZE{ 46, 52 };
        constexpr ivec2 MUZZLE{ 6, 47 }; // the ring on its left

        // the hull from the cannon to the screen's edge, so what misses the cannon doesn't fly through the ship
        constexpr ivec2 HULL_MIN{ 40, -40 };
        constexpr uvec2 HULL_SIZE{ 96, 104 };

        // the screen's top left from waterfall-front's with the cannon at its right edge, where the original stops
        // scrolling. A px higher than there, the rip's bottom edge under the ship is at 111
        constexpr ivec2 SCREEN_FROM_FRONT{ 528, -33 };

        // once he's awake the screen can still go this far back from there, and up for the platforms. That's how
        // much it moves around in a longplay of the fight
        constexpr int32_t FIGHT_SCROLL = 64;
        constexpr int32_t FIGHT_RISE   = 16;

        // waterfall-par moves with the ship in the original, it's the waterfall and the palm behind the fight
        constexpr ivec2   PAR_FROM_SHIP{ -21, 16 };
        constexpr int32_t PAR_LAYER = -119; // behind bg.1, like the rest of the far stuff
        constexpr uint8_t PAR_TICKS = 4;

        // the walkable top of the ledge, from waterfall-front's top left. What the POWs' drops land on
        constexpr int32_t LEDGE_Y = 160;

        // feet of the POWs tied up on the decks, from the ship's top left
        constexpr ivec2 POW_FEET[]{ { 85, 104 }, { 225, 104 } };

        // the screen's center from the player's feet in the arena, the original keeps him lower than in the village
        constexpr int32_t ARENA_FEET_Y = 57;

        // waterfall-front's bottom edge (x and y from its top left), the rip has nothing under it. The camera
        // can't show below where it is at the screen's right edge, so it goes up on the way to the boss
        constexpr ivec2 RIP_BOTTOM[]
        {
            { 176, 303 }, { 192, 290 }, { 240, 272 }, { 320, 267 }, { 384, 246 }, { 432, 239 },
            { 464, 227 }, { 512, 217 }, { 528, 209 }, { 560, 200 }, { 608, 193 }, { 624, 191 },
        };

        constexpr int32_t WAKE_DIST     = 40; // the player's feet from the locked screen's middle
        constexpr int32_t HULL_GAP      = 6;  // between the player's box and the cannon
        constexpr int32_t SCREEN_MARGIN = 8;

        // a pistol bullet is 1, the spray's bullets too. A fireball goes through things, so it counts once and
        // then not again for a few ticks
        constexpr int32_t  MAX_HP            = 40;
        constexpr int32_t  FIREBALL_DAMAGE   = 4;
        constexpr uint16_t FIREBALL_COOLDOWN = 20;
        constexpr uint16_t FLASH_TICKS       = 3;
        constexpr color_t  FLASH{ 255, 170, 90 }; // the original goes orange, a tint is the closest

        constexpr uint32_t KILL_POINTS = 10000;

        // per frame
        constexpr uint8_t MOVE_TICKS  = 2;
        constexpr uint8_t OPEN_TICKS  = 3;
        constexpr uint8_t FIRE_TICKS  = 3;
        constexpr uint8_t SHIP_TICKS  = 45; // between the collapse frames

        constexpr uint16_t WAKE_TICKS     = 60;
        constexpr uint16_t COOLDOWN_TICKS = 110; // closed, before it fires again or moves
        constexpr uint16_t FIRE_FRAME     = 1;  // the first one with the glow
        constexpr uint8_t  SHOTS_PER_SIDE = 2;

        constexpr uint16_t DYING_TICKS    = 300;
        constexpr uint16_t COMPLETE_TICKS = 220; // the announcer, a bit before the debrief
        constexpr uint16_t BLAST_EVERY    = 7;
        constexpr uint8_t  BLAST_TICKS    = 2;

        // the ship over waterfall-front (-97), both under the player. The shots go over him like the missiles
        constexpr int32_t SHIP_LAYER   = -96;
        constexpr int32_t CANNON_LAYER = -95;
        constexpr int32_t SHOT_LAYER   = 3;

        // shot
        constexpr float    SHOT_SPEED = 85.0f;
        constexpr uvec2    SHOT_BOX{ 16, 16 };
        constexpr uint8_t  SHOT_TICKS = 3;
        constexpr uint16_t SHOT_LIFE  = 400;
        constexpr uint16_t TRAIL_EVERY = 4;
        constexpr uint8_t  TRAIL_TICKS = 3;


        struct BossData final
        {
            ivec2     front;
            entity_id ship;
            entity_id hull;
            entity_id pows[std::size(POW_FEET)];

            int32_t  hp{ MAX_HP };
            int32_t  rise{}; // px above the low height
            uint16_t flash{};
            uint16_t fireball_cooldown{};

            uint8_t shots{};    // since the last move
            bool    high{ false };
            bool    turning_back{ false };
            bool    fired{ false };
        };


        enum class ShotState : uint8_t { Flying, COUNT };

        using ShotSM = StateMachine<ShotState>;


        ivec2 ship_pos(const ivec2 front) { return front + SHIP_FROM_FRONT; }
        ivec2 low_pos(const ivec2 front) { return ship_pos(front) + LOW_FROM_SHIP; }
        ivec2 screen_center(const ivec2 front) { return front + SCREEN_FROM_FRONT + HALF_RESOLUTION_i; }

        BossData& data(const BossSM::Ctx& c) { return c.scene.get<BossData>(c.entity); }

        // open strip's top left right now
        ivec2 anchor(const BossData& d) { return low_pos(d.front) - ivec2{ 0, d.rise }; }

        int32_t rip_bottom(const int32_t x)
        {
            if (x <= RIP_BOTTOM[0].x) return INT32_MAX;

            for (size_t i = 1; i < std::size(RIP_BOTTOM); ++i)
            {
                const ivec2 a = RIP_BOTTOM[i - 1];
                const ivec2 b = RIP_BOTTOM[i];
                if (x <= b.x) return a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x);
            }

            return std::end(RIP_BOTTOM)[-1].y;
        }

        // puts the sprite's top left where the strip it's playing wants it, and the box back on the cannon
        void place(Scene& scene, const entity_id e)
        {
            const BossData& d   = scene.get<BossData>(e);
            Sprite&         spr = scene.get<Sprite>(e);

            ivec2 top_left = anchor(d);

            switch (spr.anim<CannonAnim>())
            {
                case CannonAnim::Move: top_left += MOVE_OFF; break;
                case CannonAnim::Destroyed: top_left += DESTROYED_OFF; break;
                default: break;
            }

            Transform& tr = scene.get<Transform>(e);
            tr.position   = top_left + ivec2(spr.size() / 2u);

            Collider& col = scene.get<Collider>(e);
            col.size      = BOX_SIZE + uvec2{ 0, (uint32_t)d.rise };
            col.offset    = anchor(d) + BOX_MIN + ivec2(col.size / 2u) - tr.position;
        }

        void play(const BossSM::Ctx& c, const CannonAnim a, const uint8_t ticks, const is_reversed rev = is_reversed::no)
        {
            c.scene.get<Sprite>(c.entity).play(a, ticks, rev, should_loop::no, restart_if_same::yes);
            place(c.scene, c.entity);
        }

        bool anim_done(const BossSM::Ctx& c) { return c.scene.get<Sprite>(c.entity).anim_done(); }


        ShotState on_flying(const ShotSM::Ctx& c)
        {
            if (c.scene.get<Explosive>(c.entity).exploded || c.state.ticks >= SHOT_LIFE) return ShotState::COUNT;

            const Transform& tr = c.scene.get<Transform>(c.entity);
            if (!(c.scene.get<Sprite>(c.entity).screen_bounds(tr, c.scene.camera_pos_screen()) & aabb::screen))
                return ShotState::COUNT;

            if (c.state.ticks % TRAIL_EVERY == 0)
            {
                const vec2  v    = c.scene.get<Collider>(c.entity).velocity;
                const ivec2 tail = tr.position - ivec2(glm::normalize(v) * (float)SHOT_BOX.x);

                App::ctx().particles.emit(FxAnim::Trail, {
                    .position  = vec2(tail),
                    .frame_dur = TRAIL_TICKS,
                    .layer     = SHOT_LAYER - 1
                });
            }

            return ShotState::Flying;
        }

        void shot_end(const ShotSM::Ctx& c)
        {
            if (!c.scene.get<Explosive>(c.entity).exploded)
            {
                c.scene.destroy_entity(c.entity);
                return;
            }

            const aabb b = c.scene.get<Collider>(c.entity).bounds(c.scene.get<Transform>(c.entity));
            explode(c.scene, c.entity, { (b.min.x + b.max.x) / 2, b.max.y });
        }

        constexpr ShotSM shot_sm
        {
            ShotSM::Behaviour{ nullptr, on_flying, shot_end }, // Flying
        };

        void fire_shot(Scene& scene, const ivec2 from, const vec2 velocity)
        {
            const entity_id s = scene.create_entity();

            scene.set<Transform>(s, from);
            scene.set<Projectile>(s, ProjectileType::CannonShot);
            scene.set<Explosive>(s);
            scene.set<Sprite>(s, FxAnim::Shot, SHOT_TICKS, should_loop::yes).layer = SHOT_LAYER;

            Collider& col = scene.set<Collider>(s, ivec2{ 0 }, ivec2(SHOT_BOX));

            col.gravity      = 0.0f;
            col.velocity     = velocity;
            col.layer        |= Layer::EnemyProjectile;
            col.target_layer |= Layer::Player | Layer::TileMap;
            col.callback     = on_explosive_hit;

            shot_sm.start(scene, s, ShotState::Flying);
        }


        void on_cannon_shot(Scene& scene, const entity_id self, const entity_id other, uint32_t)
        {
            BossData& d = scene.get<BossData>(self);
            if (d.hp <= 0) return;

            if (scene.get<Projectile>(other).type == ProjectileType::Fireball)
            {
                if (d.fireball_cooldown > 0) return;

                d.fireball_cooldown = FIREBALL_COOLDOWN;
                d.hp -= FIREBALL_DAMAGE;
            }
            else --d.hp;

            d.flash = FLASH_TICKS;
        }


        // the states that can be shot in all check this first
        bool dead(const BossSM::Ctx& c) { return data(c).hp <= 0; }


        void asleep_enter(const BossSM::Ctx& c)
        {
            // the end of the turn, looking away, it plays back from here to face the player
            Sprite& spr = c.scene.get<Sprite>(c.entity);
            spr.play(CannonAnim::Move);
            spr.force_frame(anim::frame_count(anim::info(CannonAnim::Move)) - 1);
            place(c.scene, c.entity);
        }

        BossState on_asleep(const BossSM::Ctx& c)
        {
            const int32_t x = feet(c.scene, MetalSlug::user_ctx().player.legs()).x;
            return x >= screen_center(data(c).front).x - WAKE_DIST ? BossState::Wake : BossState::Asleep;
        }


        void wake_enter(const BossSM::Ctx&) { MetalSlug::user_ctx().sounds.music(Music::Boss); }

        BossState on_wake(const BossSM::Ctx& c) { return c.state.ticks >= WAKE_TICKS ? BossState::Deploy : BossState::Wake; }


        void deploy_enter(const BossSM::Ctx& c)
        {
            c.scene.get<Collider>(c.entity).enabled = true;
            play(c, CannonAnim::Move, MOVE_TICKS, is_reversed::yes);
        }

        BossState on_deploy(const BossSM::Ctx& c)
        {
            if (dead(c)) return BossState::Dying;
            return anim_done(c) ? BossState::Open : BossState::Deploy;
        }


        void open_enter(const BossSM::Ctx& c) { play(c, CannonAnim::Open, OPEN_TICKS); }

        BossState on_open(const BossSM::Ctx& c)
        {
            if (dead(c)) return BossState::Dying;
            return anim_done(c) ? BossState::Fire : BossState::Open;
        }


        void fire_enter(const BossSM::Ctx& c)
        {
            data(c).fired = false;
            play(c, CannonAnim::Fire, FIRE_TICKS);
        }

        BossState on_fire(const BossSM::Ctx& c)
        {
            if (dead(c)) return BossState::Dying;

            BossData& d = data(c);

            if (!d.fired && c.state.ticks >= FIRE_FRAME * FIRE_TICKS)
            {
                // straight ahead at both heights, the low one has to be jumped and the high one ducked under.
                // Aimed at him it was hardly dodgeable at all
                d.fired = true;
                ++d.shots;

                // creates an entity, no component references across this
                fire_shot(c.scene, anchor(d) + MUZZLE, { -SHOT_SPEED, 0.0f });
                MetalSlug::user_ctx().sounds.play(Sfx::CannonShot);
            }

            return anim_done(c) ? BossState::Close : BossState::Fire;
        }


        void close_enter(const BossSM::Ctx& c) { play(c, CannonAnim::Open, OPEN_TICKS, is_reversed::yes); }

        BossState on_close(const BossSM::Ctx& c)
        {
            if (dead(c)) return BossState::Dying;
            if (!anim_done(c) || c.state.ticks < COOLDOWN_TICKS) return BossState::Close;

            return data(c).shots >= SHOTS_PER_SIDE ? BossState::Move : BossState::Open;
        }


        void move_enter(const BossSM::Ctx& c)
        {
            BossData& d    = data(c);
            d.shots        = 0;
            d.turning_back = false;

            play(c, CannonAnim::Move, MOVE_TICKS);
        }

        BossState on_move(const BossSM::Ctx& c)
        {
            if (dead(c)) return BossState::Dying;

            BossData&      d    = data(c);
            const uint32_t last = anim::frame_count(anim::info(CannonAnim::Move)) - 1;

            if (anim_done(c))
            {
                if (d.turning_back) return BossState::Open;

                d.turning_back = true;
                d.high         = !d.high;
                play(c, CannonAnim::Move, MOVE_TICKS, is_reversed::yes);
            }

            // slides over while it's turned, halfway there when it's facing away
            const uint32_t frame = c.scene.get<Sprite>(c.entity).frame();
            const int32_t  from  = d.high == d.turning_back ? 0 : HIGH_RISE;
            const int32_t  to    = HIGH_RISE - from;
            const int32_t  step  = d.turning_back ? (int32_t)(2 * last - frame) : (int32_t)frame;

            d.rise = from + (to - from) * step / (int32_t)(2 * last);
            place(c.scene, c.entity);

            return BossState::Move;
        }


        void dying_enter(const BossSM::Ctx& c)
        {
            BossData& d = data(c);
            d.flash     = 0;

            c.scene.get<Collider>(c.entity).enabled = false;
            c.scene.get<Collider>(d.hull).enabled   = false;

            // the ones still tied up get away with the ship coming apart under them
            for (const entity_id pow : d.pows) MetalSlug::user_ctx().pows.release(c.scene, pow);

            play(c, CannonAnim::Destroyed, 0);

            MetalSlug::user_ctx().player.add_score(KILL_POINTS);
            MetalSlug::user_ctx().sounds.stop_music();
        }

        BossState on_dying(const BossSM::Ctx& c)
        {
            const uint16_t t = c.state.ticks;
            if (t >= DYING_TICKS) return BossState::Dying;

            const BossData& d = data(c);

            const uint32_t frames = anim::frame_count(anim::info(anim::boss::Ship::Destroyed));
            c.scene.get<Sprite>(d.ship).force_frame(glm::min((uint32_t)t / SHIP_TICKS, frames - 1));

            if (t == COMPLETE_TICKS) MetalSlug::user_ctx().sounds.play(Sfx::MissionComplete);
            if (t % BLAST_EVERY != 0 || t > COMPLETE_TICKS) return BossState::Dying;

            // all over the hull and the front of the ship, it's the part that comes apart
            const ivec2 ship = ship_pos(d.front);
            Random&     rng  = App::ctx().rng;
            const ivec2 at   = ship + ivec2{ rng.range(120, 360), rng.range(60, 230) };

            const anim::Vfx a = t % (3 * BLAST_EVERY) == 0 ? anim::Vfx::BigExplosion : anim::Vfx::SmallExplosion;
            App::ctx().particles.emit(a, { .position = vec2(at), .frame_dur = BLAST_TICKS, .layer = SHOT_LAYER });
            MetalSlug::user_ctx().sounds.explosion();

            return BossState::Dying;
        }


        using B = BossSM::Behaviour;
        constexpr BossSM boss_sm
        {
            B{ asleep_enter, on_asleep, nullptr }, // Asleep
            B{ wake_enter, on_wake, nullptr },     // Wake
            B{ deploy_enter, on_deploy, nullptr }, // Deploy
            B{ open_enter, on_open, nullptr },     // Open
            B{ fire_enter, on_fire, nullptr },     // Fire
            B{ close_enter, on_close, nullptr },   // Close
            B{ move_enter, on_move, nullptr },     // Move
            B{ dying_enter, on_dying, nullptr },   // Dying
        };
    }


    void Boss::register_machines() const
    {
        StateMachineSystem& sms = App::ctx().state_machines;
        sms.add(boss_sm);
        sms.add(shot_sm);
    }

    void Boss::create(Scene& scene, const ivec2 front)
    {
        front_ = front;

        // frame 0 of the collapse is the ship in one piece, the dying state steps through the rest
        const entity_id ship     = scene.create_entity();
        Transform&      ship_tr  = scene.set<Transform>(ship);
        Sprite&         ship_spr = scene.set<Sprite>(ship, anim::boss::Ship::Destroyed);
        ship_spr.layer   = SHIP_LAYER;
        ship_tr.position = ship_pos(front) + ivec2(ship_spr.size() / 2u);

        const entity_id hull = scene.create_entity();
        scene.set<Transform>(hull, low_pos(front) + HULL_MIN + ivec2(HULL_SIZE / 2u));

        // no callback, the bullets stop on anything on the Enemy layer by themselves
        Collider& hull_col             = scene.set<Collider>(hull, ivec2{ 0 }, ivec2(HULL_SIZE));
        hull_col.gravity               = 0.0f;
        hull_col.collide_with_tile_map = false;
        hull_col.layer                 |= Layer::Enemy;

        const entity_id par = scene.create_entity();
        Transform&      par_tr  = scene.set<Transform>(par);
        Sprite&         par_spr = scene.set<Sprite>(par, anim::Bg::WaterfallPar, PAR_TICKS, should_loop::yes);
        par_spr.layer   = PAR_LAYER;
        par_tr.position = ship_pos(front) + PAR_FROM_SHIP + ivec2(par_spr.size() / 2u);

        const POWManager& pows = MetalSlug::user_ctx().pows;
        entity_id         tied[std::size(POW_FEET)];

        for (size_t i = 0; i < std::size(POW_FEET); ++i)
            tied[i] = pows.spawn(scene, ship_pos(front) + POW_FEET[i], POWManager::Kind::Sitting, front.y + LEDGE_Y);

        cannon_ = scene.create_entity();
        scene.set<Transform>(cannon_);
        BossData& d = scene.set<BossData>(cannon_, front, ship, hull);
        for (size_t i = 0; i < std::size(tied); ++i) d.pows[i] = tied[i];
        scene.set<Sprite>(cannon_, CannonAnim::Move).layer = CANNON_LAYER;

        Collider& col = scene.set<Collider>(cannon_, ivec2{ 0 }, ivec2(BOX_SIZE));
        col.gravity = 0.0f;
        col.collide_with_tile_map = false;
        col.enabled = false; // until it turns around
        col.layer |= Layer::Enemy;
        col.target_layer |= Layer::PlayerProjectile;
        col.callback = on_cannon_shot;

        boss_sm.start(scene, cannon_, BossState::Asleep);
    }

    void Boss::late_update(Scene& scene) const
    {
        if (!scene.valid(cannon_)) return;

        BossData& d = scene.get<BossData>(cannon_);

        if (d.fireball_cooldown > 0) --d.fireball_cooldown;
        if (d.flash > 0) --d.flash;

        const color_t tint = d.flash > 0 ? FLASH : color_t{};
        scene.get<Sprite>(cannon_).color = scene.get<Sprite>(d.ship).color = tint;


        const entity_id legs = MetalSlug::user_ctx().player.legs();
        Transform&      tr   = scene.get<Transform>(legs);
        const aabb      box  = scene.get<Collider>(legs).bounds(tr);

        const int32_t right = low_pos(front_).x - HULL_GAP;
        if (box.max.x > right) tr.position.x -= box.max.x - right;

        if (scene.get<State<BossState>>(cannon_).current == BossState::Asleep) return;

        const int32_t left = scene.camera_pos_screen().x + SCREEN_MARGIN;
        if (box.min.x < left) tr.position.x += left - box.min.x;
    }

    ivec2 Boss::camera(const Scene& scene, ivec2 follow) const
    {
        const aabb lim = camera_limits(scene);

        if (scene.valid(cannon_) && scene.camera_pos().x >= front_.x)
            follow.y = feet(scene, MetalSlug::user_ctx().player.legs()).y - ARENA_FEET_Y;

        return glm::clamp(follow, lim.min, lim.max);
    }

    aabb Boss::camera_limits(const Scene& scene) const
    {
        // the tile map starts at the background's top left, but it can be wider than it
        const ivec2 origin = scene.get<Sprite>(scene.bg_entity).bounds(scene.get<Transform>(scene.bg_entity)).min;
        const ivec2 size   = ivec2(App::ctx().tile_map.size()) * TILE_SIZE_i;

        aabb lim{ origin + HALF_RESOLUTION_i, origin + size - HALF_RESOLUTION_i };
        if (!scene.valid(cannon_) || scene.camera_pos().x < front_.x) return lim;

        // the climb up the wreck goes over the top of the map
        const ivec2 fight = screen_center(front_);
        lim.min.y         = fight.y - FIGHT_RISE;

        if (scene.get<State<BossState>>(cannon_).current != BossState::Asleep)
        {
            lim.min.x = fight.x - FIGHT_SCROLL;
            lim.max.y = fight.y;
            return lim;
        }

        const int32_t bottom = rip_bottom(scene.camera_pos().x + HALF_RESOLUTION_i.x - front_.x);
        if (bottom != INT32_MAX) lim.max.y = glm::max(lim.min.y, front_.y + bottom - HALF_RESOLUTION_i.y);

        return lim;
    }

    bool Boss::defeated(const Scene& scene) const
    {
        if (!scene.valid(cannon_)) return false;

        const State<BossState>& s = scene.get<State<BossState>>(cannon_);
        return s.current == BossState::Dying && s.ticks >= DYING_TICKS;
    }
}
