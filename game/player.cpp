#include "mse/mse.hpp"
#include "player.hpp"
#include "player_anim.hpp"
#include "layer.hpp"

using namespace mse;
using namespace mse::literals;
using namespace mse::anim::player;


namespace player
{
    namespace
    {
        struct WeaponData final
        {
            uint16_t refire_ticks; // fire cooldown
            uint8_t  burst;        // bullets per press
            uint8_t  burst_ticks;  // coolldown between the bullets of a burst
            float    speed;
        };

        // indexed by WeaponType
        constexpr WeaponData WEAPONS[]
        {
            {  6, 1, 0, 250.0f }, // Pistol
            {  4, 4, 4, 300.0f }, // RifleSpray, a press mid burst starts a new one
            { 18, 1, 0, 180.0f }, // RifleFireball, 18 = its whole shoot animation
        };

        static_assert(std::size(WEAPONS) == (size_t)WeaponType::Count);
        static_assert(std::size(POSE_ANIM) == (size_t)WeaponType::Count);

        // per bullet of a burst, so the spray isn't a straight line
        const float SPRAY_SPREAD[]{ 0.0_deg, 4.0_deg, -3.0_deg, 2.0_deg };

        constexpr uint8_t FIREBALL_CREATE_TICKS = 2;
        constexpr uint8_t FIREBALL_TICKS        = 2;
        constexpr uint8_t FIREBALL_EXPIRE_TICKS = 2;
    }


    struct Player::BodyStateManager final
    {
        // raising and lowering the gun. Not while crouching, down is held then
        static bool look_changed(const BodySM::Ctx& c)
        {
            const entity_id l  = legs_of(c);
            const bool      up = c.scene.get<Intent>(l).dir.y < 0;

            BodyData& bd = c.scene.get<BodyData>(c.entity);
            if (up == bd.was_up) return false;

            bd.was_up = up;
            if (c.scene.get<State<LegsState>>(l).current == LegsState::Crouch) return false;

            bd.transition = up ? Pose::LookUp : Pose::LookUpEnd;
            bd.restart    = true;
            return true;
        }

        static void note_fire(const BodySM::Ctx& c)
        {
            if (c.scene.get<Intent>(legs_of(c)).just_pressed & Intent::Fire)
                c.scene.get<BodyData>(c.entity).fire_queued = true;
        }

        // true while the animation that was asked for hasn't been played yet (late_update does it, after the machines)
        static bool anim_pending(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).restart; }

        // crouch shoot chains into after shoot, so the shot isn't over when the first one is
        static bool shot_done(const BodySM::Ctx& c)
        {
            const Sprite& sprite = c.scene.get<Sprite>(c.entity);
            return sprite.anim_done() && data((Body)sprite.info().anim_id).end != End::Next;
        }

        static void fire(const BodySM::Ctx& c)
        {
            const entity_id l  = legs_of(c);
            const Intent    in = c.scene.get<Intent>(l);
            const bool      air = c.scene.get<State<LegsState>>(l).current == LegsState::Air;

            BodyData& bd = c.scene.get<BodyData>(c.entity);
            LegsData& ld = c.scene.get<LegsData>(l);

            // locked here so the bullet and the animation agree if up is released halfway
            if (bd.burst_shot == 0)
                bd.aim = in.dir.y < 0 ? Aim::Up : in.dir.y > 0 && air ? Aim::Down : Aim::Forward;

            // landed mid burst
            if (bd.aim == Aim::Down && !air) bd.aim = Aim::Forward;

            bd.restart = true;

            // Player::shoot
            shoot(c.scene, c.entity, bd.aim, ld.facing);

            ++bd.burst_shot;
            --bd.burst_left;
        }

        static void start_burst(const BodySM::Ctx& c)
        {
            BodyData& bd = c.scene.get<BodyData>(c.entity);

            bd.fire_queued = false;
            bd.burst_left  = WEAPONS[(size_t)bd.weapon].burst;
            bd.burst_shot  = 0;

            fire(c);
            c.state.ticks = 0;
        }


        static BodyState ready(const BodySM::Ctx& c)
        {
            note_fire(c);
            if (look_changed(c)) return BodyState::Transition;

            return c.scene.get<BodyData>(c.entity).fire_queued ? BodyState::Shooting : BodyState::Ready;
        }

        static BodyState shooting(const BodySM::Ctx& c)
        {
            note_fire(c);

            BodyData&         bd  = c.scene.get<BodyData>(c.entity);
            const WeaponData& gun = WEAPONS[(size_t)bd.weapon];

            // a burst isn't cut by raising or lowering the gun, the aim is locked for it anyway
            if (bd.burst_left == 0 && look_changed(c)) return BodyState::Transition;

            if (bd.fire_queued && c.state.ticks >= gun.refire_ticks)
            {
                start_burst(c);
                return BodyState::Shooting;
            }

            if (bd.burst_left > 0)
            {
                if (c.state.ticks >= gun.burst_ticks)
                {
                    fire(c);
                    c.state.ticks = 0;
                }

                return BodyState::Shooting;
            }

            return !anim_pending(c) && shot_done(c) ? BodyState::Ready : BodyState::Shooting;
        }

        // Turning and changing level (crouch <-> stand <-> looking up) have their own sprites.
        // Nothing is fired until the sprite is done, that's all this state is for
        static BodyState transition(const BodySM::Ctx& c)
        {
            note_fire(c);

            const entity_id l  = legs_of(c);
            const BodyData& bd = c.scene.get<BodyData>(c.entity);

            // the only one that can be interrupted
            if (bd.transition == Pose::WalkStop)
            {
                if (c.scene.get<State<LegsState>>(l).current != LegsState::Stand) return BodyState::Ready;
                if (bd.fire_queued) return BodyState::Shooting;
            }

            if (anim_pending(c) || !c.scene.get<Sprite>(c.entity).anim_done()) return BodyState::Transition;

            return bd.fire_queued ? BodyState::Shooting : BodyState::Ready;
        }
    };

    struct Player::LegsStateManager final
    {

        static BodyState body_state(const LegsSM::Ctx& c)
        {
            return c.scene.get<State<BodyState>>(body_of(c)).current;
        }


        static void stand_enter(const LegsSM::Ctx& c)
        {
            play(c.scene.get<Sprite>(c.entity), Legs::Idle);
            c.scene.get<Collider>(c.entity).velocity.x = 0.0f;

            // doesn't interrupt a shot
            if (c.state.previous == LegsState::Walk && body_state(c) == BodyState::Ready)
                begin_transition(
                    c.scene, body_of(c), Pose::WalkStop);
        }

        static void walk_enter(const LegsSM::Ctx& c) { play(c.scene.get<Sprite>(c.entity), Legs::Walk); }

        static void air_enter(const LegsSM::Ctx& c)
        {
            const Intent in = c.scene.get<Intent>(c.entity);

            // side jump only when the jump starts while moving, not for falling or steering later
            const bool side = (in.just_pressed & Intent::Jump) && in.dir.x != 0;
            play(c.scene.get<Sprite>(c.entity), side ? Legs::SideJump : Legs::InAir);

            // the turns and the crouch transitions are full body sprites standing on the ground, drop them.
            // raising / lowering the gun is only the torso and carries on
            const entity_id body = body_of(c);
            const Pose      t    = c.scene.get<BodyData>(body).transition;

            if (body_state(c) == BodyState::Transition && t != Pose::LookUp && t != Pose::LookUpEnd)
                body_sm().force(
                    c.scene, body, BodyState::Ready);
        }

        // the crouch sprites are full body, late_update hides the legs for those
        static void crouch_enter(const LegsSM::Ctx& c)
        {
            begin_transition(c.scene, body_of(c), Pose::CrouchDown);
        }

        // also when leaving for Air, air_enter runs right after and drops it
        static void crouch_exit(const LegsSM::Ctx& c)
        {
            begin_transition(c.scene, body_of(c), Pose::CrouchUp);
        }


        // what Stand, Walk and Crouch share
        static LegsState grounded(const LegsSM::Ctx& c, const float speed, const bool can_jump = true)
        {
            const Intent in  = c.scene.get<Intent>(c.entity);
            Collider&    col = c.scene.get<Collider>(c.entity);

            col.velocity.x = (float)in.dir.x * speed;

            // facing only changes here: steering in the air is fine, turning around isn't
            if (LegsData& ld = c.scene.get<LegsData>(c.entity);
                in.dir.x != 0 && in.dir.x != ld.facing)
            {
                ld.facing = in.dir.x;

                Pose turn = Pose::Turn;
                if (c.state.current == LegsState::Crouch) turn = Pose::CrouchTurn;
                else if (in.dir.y < 0) turn = Pose::LookUpTurn;

                begin_transition(c.scene, body_of(c), turn);
            }

            if (!col.grounded) return LegsState::Air;

            // the impulse is here and not in air_enter, walking off a ledge enters Air too
            if (can_jump && (in.just_pressed & Intent::Jump))
            {
                col.velocity.y = -JUMP_VELOCITY;
                return LegsState::Air;
            }

            if (in.dir.y > 0) return LegsState::Crouch;
            return in.dir.x != 0 ? LegsState::Walk : LegsState::Stand;
        }

        static LegsState stand(const LegsSM::Ctx& c) { return grounded(c, 0.0f); }
        static LegsState walk(const LegsSM::Ctx& c) { return grounded(c, WALK_SPEED); }

        static LegsState crouch(const LegsSM::Ctx& c)
        {
            const Intent in = c.scene.get<Intent>(c.entity);

            // no crawling or jumping while shooting, only turning and standing up.
            // the press counts too, the body only switches to Shooting after the legs have run
            const bool shooting = body_state(c) == BodyState::Shooting || (in.just_pressed & Intent::Fire);

            return grounded(c, shooting ? 0.0f : CRAWL_SPEED, !shooting);
        }

        static LegsState air(const LegsSM::Ctx& c)
        {
            const Intent in  = c.scene.get<Intent>(c.entity);
            Collider&    col = c.scene.get<Collider>(c.entity);

            col.velocity.x = (float)in.dir.x * WALK_SPEED;

            if (!col.grounded)
            {
                // looking down doesn't go with the side jump legs. They stay the normal ones for the rest of the jump
                if (Sprite& sprite = c.scene.get<Sprite>(c.entity);
                    in.dir.y > 0 && (Legs)sprite.info().anim_id == Legs::SideJump)
                    play(sprite, Legs::InAir);

                return LegsState::Air;
            }

            if (in.dir.y > 0) return LegsState::Crouch;
            return in.dir.x != 0 ? LegsState::Walk : LegsState::Stand;
        }
    };


    const BodySM& Player::body_sm()
    {
        // enter doesn't play anything, the animation depends on the legs and the weapon too (see pick_pose)
        using BSM = BodyStateManager;

        static constexpr BodySM body_sm
        {
            BodySM::Behaviour{ nullptr, BSM::ready, nullptr },              // Ready
            BodySM::Behaviour{ BSM::start_burst, BSM::shooting, nullptr },  // Shooting
            BodySM::Behaviour{ nullptr, BSM::transition, nullptr },         // Transition
        };

        return body_sm;
    }

    const LegsSM& Player::legs_sm()
    {
        using LSM = LegsStateManager;
        static constexpr LegsSM legs_sm
        {
            LegsSM::Behaviour{ LSM::stand_enter, LSM::stand, nullptr },            // Stand
            LegsSM::Behaviour{ LSM::walk_enter, LSM::walk, nullptr },              // Walk
            LegsSM::Behaviour{ LSM::air_enter, LSM::air, nullptr },                // Air
            LegsSM::Behaviour{ LSM::crouch_enter, LSM::crouch, LSM::crouch_exit }, // Crouch
        };

        return legs_sm;
    }


    void Player::register_machines()
    {
        // legs must be first, the body reads their state
        StateMachineSystem& sms = App::ctx().state_machines;
        sms.add(legs_sm());
        sms.add(body_sm());
    }


    Player::Player(Scene& scene)
    {
        legs_ = scene.create_entity();
        body_ = scene.create_entity();

        scene.set<Transform>(legs_, ivec2{ 100, -50 });
        scene.set<Transform>(body_);

        scene.set<Sprite>(legs_, Legs::Idle, should_loop::yes);
        scene.set<Sprite>(body_, Body::PistolIdle, should_loop::yes).layer = 1;

        Collider& legs_col = scene.set<Collider>(legs_);
        legs_col.layer     = (uint32_t)Layer::Player;
        legs_col.gravity   = JUMP_GRAVITY;
        legs_col.size      = LEGS_SIZE;

        // sized to the body sprite in late_update
        Collider& body_col = scene.set<Collider>(body_);
        body_col.layer     = (uint32_t)Layer::Player;

        body_col.gravity               = 0.0f;
        body_col.collide_with_tile_map = false;

        scene.set<Intent>(legs_);
        scene.set<LegsData>(legs_, body_);
        scene.set<BodyData>(body_, legs_);


        for (size_t i = 0; i < BULLET_POOL_SIZE; ++i)
        {
            const entity_id bullet = scene.create_entity();

            scene.set<Transform>(bullet);
            scene.set<Sprite>(bullet, anim::Bullet::Pistol, should_loop::no).hidden = true;
            scene.set<PlayerBullet>(bullet, WeaponType::Pistol);

            Collider& col = scene.set<Collider>(bullet);
            col.enabled = false;
            col.gravity = 0.0f;
            col.target_layer |= Layer::TileMap | Layer::POW;
            col.layer |= Layer::PlayerBullet;

            col.callback = +[](Scene& scene, const entity_id self, entity_id, const uint32_t layer)
            {
                const PlayerBullet b = scene.get<PlayerBullet>(self);
                if (layer & Layer::TileMap || b.type != WeaponType::RifleFireball) stop_bullet(scene, self, b);
            };
        }

        legs_sm().start(scene, legs_, LegsState::Stand);
        body_sm().start(scene, body_, BodyState::Ready);
    }


    void Player::update(Scene& scene) const
    {
        Intent& in = scene.set<Intent>(legs_); // resets the intent

        if (Input::down(Key::A)) --in.dir.x;
        if (Input::down(Key::D)) ++in.dir.x;
        if (Input::down(Key::W)) --in.dir.y;
        if (Input::down(Key::S)) ++in.dir.y;

        if (Input::down(Key::Space)) in.down |= Intent::Jump;
        if (Input::down(Key::J)) in.down |= Intent::Fire;

        if (Input::just_pressed(Key::Space)) in.just_pressed |= Intent::Jump;
        if (Input::just_pressed(Key::J)) in.just_pressed |= Intent::Fire;

        // for testing until the pickups exist
        BodyData&        bd     = scene.get<BodyData>(body_);
        const WeaponType before = bd.weapon;

        if (Input::just_pressed(Key::Num1)) bd.weapon = WeaponType::Pistol;
        if (Input::just_pressed(Key::Num2)) bd.weapon = WeaponType::RifleSpray;
        if (Input::just_pressed(Key::Num3)) bd.weapon = WeaponType::RifleFireball;

        // the rest of a spray burst would come out of the new gun at its burst_ticks
        if (bd.weapon != before) bd.burst_left = 0;
    }

    void Player::late_update(Scene& scene) const
    {
        Transform& legs_tr  = scene.get<Transform>(legs_);
        Sprite&    legs_spr = scene.get<Sprite>(legs_);
        Collider&  legs_col = scene.get<Collider>(legs_);
        LegsData&  ld       = scene.get<LegsData>(legs_);

        Transform& body_tr  = scene.get<Transform>(body_);
        Sprite&    body_spr = scene.get<Sprite>(body_);
        Collider&  body_col = scene.get<Collider>(body_);
        BodyData&  bd       = scene.get<BodyData>(body_);

        const Intent in        = scene.get<Intent>(legs_);
        const Legs   legs_anim = (Legs)legs_spr.info().anim_id;


        // body animation

        const Pose pose = pick_pose(
            scene.get<State<BodyState>>(body_).current,
            scene.get<State<LegsState>>(legs_),
            in,
            legs_anim == Legs::SideJump,
            bd);

        const PoseAnim wanted = POSE_ANIM[(size_t)bd.weapon][(size_t)pose];

        if (wanted != bd.requested || bd.restart)
        {
            // a crouch shot has already chained into its after shoot when it ends, that one just stays
            const bool showing = !bd.restart && !wanted.reversed && (Body)body_spr.info().anim_id == wanted.anim;
            if (!showing) play(body_spr, wanted.anim, wanted.reversed);

            bd.requested = wanted;
            bd.restart   = false;
        }
        else if (const BodyAnim playing = data((Body)body_spr.info().anim_id);
            body_spr.anim_done() && playing.end == End::Next) { play(body_spr, playing.next); }

        bd.last_pose = pose;

        const Body     body_anim = (Body)body_spr.info().anim_id;
        const BodyAnim body_data = data(body_anim);
        const LegsAnim legs_data = data(legs_anim);
        const bool     full      = body_data.full_body;


        // placement

        const bool flip = ld.facing < 0;
        legs_spr.flip_x = body_spr.flip_x = flip;
        legs_spr.hidden = full;

        const ivec2 center = legs_tr.position - ld.sprite_offset;

        const ivec2 legs_size = legs_spr.size() * legs_tr.scale;
        const ivec2 body_size = body_spr.size() * body_tr.scale;

        legs_tr.position = sprite_center(center, legs_data.origin, legs_size, flip);
        ld.sprite_offset = legs_tr.position - center;

        legs_col.offset = -ld.sprite_offset;
        legs_col.size.y = full ? LEGS_SMALL_HEIGHT : LEGS_SIZE.y;
        if (full) legs_col.offset.y += LEGS_SMALL_OFFSET;

        ivec2 body_origin = body_data.origin;
        if (!full) body_origin.y -= legs_data.waist_lift;

        body_tr.position = sprite_center(center, body_origin, body_size, flip);
        body_col.size    = body_size;

        scene.get<Transform>(scene.camera()).position = center + CAMERA_OFFSET;

        update_bullets(scene);
    }


    void Player::update_bullets(Scene& scene) const
    {
        for (const entity_id bullet : scene.pool<PlayerBullet>().owners())
        {
            Sprite& spr = scene.get<Sprite>(bullet);
            if (spr.hidden) continue;

            if (scene.get<PlayerBullet>(bullet).type == WeaponType::RifleFireball)
            {
                const anim::Bullet playing = (anim::Bullet)spr.info().anim_id;

                // Free again once it has burnt out
                if (playing == anim::Bullet::RifleFireballExpire)
                {
                    spr.hidden = spr.anim_done();
                    continue;
                }

                if (playing == anim::Bullet::RifleFireballCreate && spr.anim_done())
                    spr.play(anim::Bullet::RifleFireball, FIREBALL_TICKS, is_reversed::no, should_loop::yes);
            }

            const Transform& tr = scene.get<Transform>(bullet);
            if (spr.screen_bounds(tr, scene.camera_pos_screen()) & aabb::screen) continue;

            scene.get<Collider>(bullet).enabled = false;
            spr.hidden = true;
        }
    }


    // hit a tile or something it targets
    void Player::stop_bullet(Scene& scene, const entity_id bullet, const PlayerBullet type)
    {
        Collider& col = scene.get<Collider>(bullet);
        col.enabled   = false;
        col.velocity  = vec2{ 0.0f };

        Sprite& spr = scene.get<Sprite>(bullet);

        // hidden by update_bullets when it's done
        if (type.type != WeaponType::RifleFireball) spr.hidden = true;
        else spr.play(anim::Bullet::RifleFireballExpire, FIREBALL_EXPIRE_TICKS);
    }


    void Player::shoot(Scene& scene, const entity_id body, const Aim aim, const int8_t dir)
    {
        BodyData&       data   = scene.get<BodyData>(body);
        const entity_id bullet = take_bullet(scene, data);

        const WeaponType  type = data.weapon;
        const WeaponData& gun  = WEAPONS[(size_t)type];

        scene.get<PlayerBullet>(bullet).type = type;

        Transform& tr  = scene.get<Transform>(bullet);
        Sprite&    spr = scene.get<Sprite>(bullet);
        Collider&  col = scene.get<Collider>(bullet);

        spr.hidden  = false;
        col.enabled = true;

        if (type == WeaponType::RifleFireball)
            spr.play(anim::Bullet::RifleFireballCreate, FIREBALL_CREATE_TICKS,
                     is_reversed::no, should_loop::no, restart_if_same::yes);
        else spr.play(anim::Bullet::Pistol); // TODO: find spray bullet, uses the pistol bullet for now


        const ivec2 aim_dir = [aim, dir]
        {
            switch (aim)
            {
                case Aim::Forward: return ivec2{ dir, 0 };
                case Aim::Up: return ivec2{ 0, -1 };
                case Aim::Down: return ivec2{ 0, 1 };
                default: assert(!"not implemented");
            }

            return ivec2{};
        }();

        vec2 vel = vec2(aim_dir) * gun.speed;
        if (type == WeaponType::RifleSpray)
            vel = glm::rotate(vel, SPRAY_SPREAD[(size_t)data.burst_shot % std::size(SPRAY_SPREAD)]);

        tr.position  = scene.get<Transform>(body).position + aim_dir * SHOOT_OFFSET;
        col.velocity = vel;

        // the sprites point right and y points down, so a positive angle turns them down.
        // Left is a flip and not a half turn, otherwise the fireball would be upside down
        // TODO: make exported perpendicular (and 45deg for spray) sprites for bullets to have meaningful pp checks
        spr.flip_x  = vel.x < 0.0f;
        tr.rotation = glm::atan(vel.y, glm::abs(vel.x));
        if (spr.flip_x) tr.rotation = -tr.rotation;
    }

    entity_id Player::take_bullet(Scene& scene, BodyData& data)
    {
        const vector<entity_id>& bullets = scene.pool<PlayerBullet>().owners();

        for (uint32_t i = 0; i < BULLET_POOL_SIZE; ++i)
        {
            const uint32_t slot = (data.next_bullet + i) % BULLET_POOL_SIZE;

            // a fireball that hit something is still burning out with its collider off
            if (!scene.get<Collider>(bullets[slot]).enabled && scene.get<Sprite>(bullets[slot]).hidden)
            {
                data.next_bullet = (slot + 1) % BULLET_POOL_SIZE;
                return bullets[slot];
            }
        }

        const entity_id e = bullets[data.next_bullet];
        data.next_bullet  = (data.next_bullet + 1) % BULLET_POOL_SIZE;
        return e;
    }


    entity_id Player::legs_of(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).legs; }
    entity_id Player::body_of(const LegsSM::Ctx& c) { return c.scene.get<LegsData>(c.entity).body; }


    void Player::begin_transition(Scene& scene, const entity_id body, const Pose pose)
    {
        BodyData& bd  = scene.get<BodyData>(body);
        bd.transition = pose;
        bd.restart    = true;

        body_sm().force(scene, body, BodyState::Transition);
        scene.get<State<BodyState>>(body).ticks = 0;
    }

    Pose Player::pick_pose(
        const BodyState         body,
        const State<LegsState>& legs,
        const Intent            in,
        const bool              side_jump,
        const BodyData&         bd)
    {
        if (body == BodyState::Transition) return bd.transition;

        const bool crouching = legs.current == LegsState::Crouch;
        const bool in_air    = legs.current == LegsState::Air;

        if (body == BodyState::Shooting)
        {
            if (crouching) return Pose::CrouchShoot;
            if (bd.aim == Aim::Up) return Pose::ShootUp;
            if (bd.aim == Aim::Down) return Pose::ShootDown;
            return in_air ? Pose::JumpShoot : Pose::Shoot;
        }

        if (crouching)
        {
            if (in.dir.x != 0) return Pose::CrouchWalk;

            // the after shoot pose is the idle one until anything else happens
            if (bd.last_pose == Pose::CrouchShoot || bd.last_pose == Pose::CrouchAfterShoot) return Pose::CrouchAfterShoot;
            return Pose::CrouchIdle;
        }

        if (in.dir.y < 0) return Pose::LookUpIdle;

        if (in_air)
        {
            if (in.dir.y > 0) return Pose::LookDown;
            return side_jump ? Pose::SideJump : Pose::InAir;
        }

        return legs.current == LegsState::Walk ? Pose::Walk : Pose::Idle;
    }


    ivec2 Player::sprite_center(const ivec2 box_center, const ivec2 origin, const ivec2 size, const bool flip)
    {
        const ivec2 box_min = box_center - ivec2(LEGS_SIZE / 2u);
        const ivec2 box_max = box_min + ivec2(LEGS_SIZE);

        // mirrored around the box when facing left
        const int32_t left = flip ? box_max.x - origin.x - size.x : box_min.x + origin.x;

        return ivec2{ left, box_min.y + origin.y } + size / 2;
    }
}
