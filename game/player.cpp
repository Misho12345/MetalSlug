#include "mse/mse.hpp"
#include "player.hpp"
#include "player_anim.hpp"
#include "layer.hpp"

using namespace mse;
using namespace mse::literals;
using namespace mse::anim::player;


namespace player
{
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
            const LegsState ls = c.scene.get<State<LegsState>>(l).current;

            BodyData& bd = c.scene.get<BodyData>(c.entity);
            LegsData& ld = c.scene.get<LegsData>(l);

            // locked here so the bullet and the animation agree if up is released halfway
            bd.aim     = in.dir.y < 0 ? Aim::Up : in.dir.y > 0 && ls == LegsState::Air ? Aim::Down : Aim::Forward;
            bd.restart = true;

            // Player::shoot
            shoot(c.scene, c.entity, bd.aim, ld.facing);
        }


        static BodyState ready(const BodySM::Ctx& c)
        {
            if (look_changed(c)) return BodyState::Transition;

            const Intent in = c.scene.get<Intent>(legs_of(c));
            return (in.down & Intent::Fire) ? BodyState::Shooting : BodyState::Ready;
        }

        static BodyState shooting(const BodySM::Ctx& c)
        {
            if (look_changed(c)) return BodyState::Transition;

            const Intent in = c.scene.get<Intent>(legs_of(c));

            if (((in.just_pressed & Intent::Fire) && c.state.ticks >= REFIRE_TICKS) ||
                ((in.down & Intent::Fire) && c.state.ticks >= AUTO_FIRE_TICKS))
            {
                fire(c);
                c.state.ticks = 0;
                return BodyState::Shooting;
            }

            return !anim_pending(c) && shot_done(c) ? BodyState::Ready : BodyState::Shooting;
        }

        // Turning and changing level (crouch <-> stand <-> looking up) have their own sprites.
        // Nothing is fired until the sprite is done, that's all this state is for
        static BodyState transition(const BodySM::Ctx& c)
        {
            const entity_id l  = legs_of(c);
            const Intent    in = c.scene.get<Intent>(l);
            const BodyData& bd = c.scene.get<BodyData>(c.entity);

            const bool fire_held = (in.down & Intent::Fire) != 0;

            // the only one that can be interrupted
            if (bd.transition == Pose::WalkStop)
            {
                if (c.scene.get<State<LegsState>>(l).current != LegsState::Stand) return BodyState::Ready;
                if (fire_held) return BodyState::Shooting;
            }

            if (anim_pending(c) || !c.scene.get<Sprite>(c.entity).anim_done()) return BodyState::Transition;

            return fire_held ? BodyState::Shooting : BodyState::Ready;
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
            const bool shooting = body_state(c) == BodyState::Shooting || (in.down & Intent::Fire);

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
            BodySM::Behaviour{ nullptr, BSM::ready, nullptr },      // Ready
            BodySM::Behaviour{ BSM::fire, BSM::shooting, nullptr }, // Shooting
            BodySM::Behaviour{ nullptr, BSM::transition, nullptr }, // Transition
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

            col.callback = +[](Scene& scene, const entity_id self, entity_id, const uint32_t)
            {
                scene.get<Collider>(self).enabled = false;
                scene.get<Sprite>(self).hidden = true;
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
            play(body_spr, wanted.anim, wanted.reversed);

            bd.requested = wanted;
            bd.restart   = false;
        }
        else if (const BodyAnim playing = data((Body)body_spr.info().anim_id);
            body_spr.anim_done() && playing.end == End::Next) { play(body_spr, playing.next); }

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
            Collider& col = scene.get<Collider>(bullet);
            if (!col.enabled) continue;

            Transform& tr = scene.get<Transform>(bullet);
            Sprite& spr = scene.get<Sprite>(bullet);

            if (spr.screen_bounds(tr, scene.camera_pos_screen()) & aabb::screen) continue;

            col.enabled = false;
            spr.hidden  = true;
        }
    }


    void Player::shoot(Scene& scene, const entity_id body, const Aim aim, const int8_t dir)
    {
        BodyData&       data   = scene.get<BodyData>(body);
        const entity_id bullet = take_bullet(scene, data);

        if (data.weapon != scene.get<PlayerBullet>(bullet).type)
            assert(!"TODO: change");

        Transform& tr  = scene.get<Transform>(bullet);
        Sprite&    spr = scene.get<Sprite>(bullet);
        Collider&  col = scene.get<Collider>(bullet);

        spr.hidden  = false;
        col.enabled = true;


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

        tr.position  = scene.get<Transform>(body).position + aim_dir * SHOOT_OFFSET;
        tr.rotation = aim != Aim::Forward ? 90.0_deg : 0.0f;

        col.velocity = vec2(aim_dir) * SHOOT_FORCE;
    }

    entity_id Player::take_bullet(Scene& scene, BodyData& data)
    {
        const vector<entity_id>& bullets = scene.pool<PlayerBullet>().owners();

        for (uint32_t i = 0; i < BULLET_POOL_SIZE; ++i)
        {
            const uint32_t slot = (data.next_bullet + i) % BULLET_POOL_SIZE;
            if (!scene.get<Collider>(bullets[slot]).enabled)
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

        if (crouching) return in.dir.x != 0 ? Pose::CrouchWalk : Pose::CrouchIdle;

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
