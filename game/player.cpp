#include "mse/mse.hpp"
#include "player.hpp"
#include "player_anim.hpp"
#include "layer.hpp"

using namespace mse;
using namespace mse::anim::player;
using namespace player_anim;

using LegsSM = StateMachine<LegsState>;
using BodySM = StateMachine<BodyState>;

namespace
{
    // ----- Move -----

    constexpr float JUMP_HEIGHT    = 56.0f;
    constexpr float JUMP_APEX_TIME = 0.36f; // from takeoff to the highest point

    constexpr float JUMP_GRAVITY  = 2.0f * JUMP_HEIGHT / (JUMP_APEX_TIME * JUMP_APEX_TIME);
    constexpr float JUMP_VELOCITY = 2.0f * JUMP_HEIGHT / JUMP_APEX_TIME;

    constexpr float WALK_SPEED  = 100.0f;
    constexpr float CRAWL_SPEED = 50.0f;

    constexpr uint16_t REFIRE_TICKS    = 6;  // on press
    constexpr uint16_t AUTO_FIRE_TICKS = 12; // holding the button


    // ----- Colliders -----

    constexpr uvec2 LEGS_SIZE{ 21, 16 };

    // legs collider for tilemap for when hidden
    constexpr uint32_t LEGS_SMALL_HEIGHT = 4;

    // keeps the bottom of the small box where the bottom of the full one is
    constexpr int32_t LEGS_SMALL_OFFSET = (int32_t)(LEGS_SIZE.y / 2) - (int32_t)(LEGS_SMALL_HEIGHT / 2);

    constexpr ivec2 CAMERA_OFFSET{ 0, -22 };


    // ----- Components -----

    enum class Aim : uint8_t { Forward, Up, Down };

    struct LegsData final
    {
        entity_id body;
        ivec2     sprite_offset{ 0 };
        int8_t    facing{ 1 };
    };

    struct BodyData final
    {
        entity_id legs;

        PoseAnim requested{ Body::_COUNT };

        Pose       transition{ Pose::Count };
        Aim        aim{};
        WeaponType weapon{};

        bool restart{ false };
        bool was_up{ false };
    };


    // ----- Body -----

    entity_id legs_of(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).legs; }

    // raising and lowering the gun. Not while crouching, down is held then
    bool look_changed(const BodySM::Ctx& c)
    {
        const entity_id l  = legs_of(c);
        const bool      up = c.scene.get<Player::Intent>(l).dir.y < 0;

        BodyData& bd = c.scene.get<BodyData>(c.entity);
        if (up == bd.was_up) return false;

        bd.was_up = up;
        if (c.scene.get<State<LegsState>>(l).current == LegsState::Crouch) return false;

        bd.transition = up ? Pose::LookUp : Pose::LookUpEnd;
        bd.restart    = true;
        return true;
    }

    // true while the animation that was asked for hasn't been played yet (late_update does it, after the machines)
    bool anim_pending(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).restart; }

    // crouch shoot chains into after shoot, so the shot isn't over when the first one is
    bool shot_done(const BodySM::Ctx& c)
    {
        const Sprite& sprite = c.scene.get<Sprite>(c.entity);
        return sprite.anim_done() && data((Body)sprite.info().anim_id).end != End::Next;
    }

    void fire(const BodySM::Ctx& c)
    {
        const entity_id      l  = legs_of(c);
        const Player::Intent in = c.scene.get<Player::Intent>(l);

        BodyData& bd = c.scene.get<BodyData>(c.entity);

        // locked here so the bullet and the animation agree if up is released halfway
        if (in.dir.y < 0) bd.aim = Aim::Up;
        else if (in.dir.y > 0 && c.scene.get<State<LegsState>>(l).current == LegsState::Air) bd.aim = Aim::Down;
        else bd.aim = Aim::Forward;

        bd.restart = true;

        // TODO: spawn the bullet (bd.aim, LegsData::facing for Forward)
    }


    BodyState ready(const BodySM::Ctx& c)
    {
        if (look_changed(c)) return BodyState::Transition;

        const Player::Intent in = c.scene.get<Player::Intent>(legs_of(c));
        return (in.down & Player::Intent::Fire) ? BodyState::Shooting : BodyState::Ready;
    }

    BodyState shooting(const BodySM::Ctx& c)
    {
        if (look_changed(c)) return BodyState::Transition;

        const Player::Intent in = c.scene.get<Player::Intent>(legs_of(c));

        if (((in.just_pressed & Player::Intent::Fire) && c.state.ticks >= REFIRE_TICKS) ||
            ((in.down & Player::Intent::Fire) && c.state.ticks >= AUTO_FIRE_TICKS))
        {
            fire(c);
            c.state.ticks = 0;
            return BodyState::Shooting;
        }

        return !anim_pending(c) && shot_done(c) ? BodyState::Ready : BodyState::Shooting;
    }

    // Turning and changing level (crouch <-> stand <-> looking up) have their own sprites.
    // Nothing is fired until the sprite is done, that's all this state is for
    BodyState transition(const BodySM::Ctx& c)
    {
        const entity_id      l  = legs_of(c);
        const Player::Intent in = c.scene.get<Player::Intent>(l);
        const BodyData&      bd = c.scene.get<BodyData>(c.entity);

        const bool fire_held = (in.down & Player::Intent::Fire) != 0;

        // the only one that can be interrupted
        if (bd.transition == Pose::WalkStop)
        {
            if (c.scene.get<State<LegsState>>(l).current != LegsState::Stand) return BodyState::Ready;
            if (fire_held) return BodyState::Shooting;
        }

        if (anim_pending(c) || !c.scene.get<Sprite>(c.entity).anim_done()) return BodyState::Transition;

        return fire_held ? BodyState::Shooting : BodyState::Ready;
    }

    // enter doesn't play anything, the animation depends on the legs and the weapon too (see pick_pose)
    const BodySM body_sm
    {
        BodySM::Behaviour{ nullptr, ready, nullptr },      // Ready
        BodySM::Behaviour{ fire, shooting, nullptr },      // Shooting
        BodySM::Behaviour{ nullptr, transition, nullptr }, // Transition
    };

    // for the legs: they run first, so the body is already in Transition when its own stay runs this tick
    void begin_transition(Scene& scene, const entity_id body, const Pose pose)
    {
        BodyData& bd = scene.get<BodyData>(body);
        bd.transition = pose;
        bd.restart    = true;

        body_sm.force(scene, body, BodyState::Transition);
        scene.get<State<BodyState>>(body).ticks = 0;
    }

    Pose pick_pose(
        const BodyState         body,
        const State<LegsState>& legs,
        const Player::Intent    in,
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


    // ----- Legs -----

    entity_id body_of(const LegsSM::Ctx& c) { return c.scene.get<LegsData>(c.entity).body; }

    BodyState body_state(const LegsSM::Ctx& c) { return c.scene.get<State<BodyState>>(body_of(c)).current; }


    void stand_enter(const LegsSM::Ctx& c)
    {
        play(c.scene.get<Sprite>(c.entity), Legs::Idle);
        c.scene.get<Collider>(c.entity).velocity.x = 0.0f;

        // doesn't interrupt a shot
        if (c.state.previous == LegsState::Walk && body_state(c) == BodyState::Ready)
            begin_transition(c.scene, body_of(c), Pose::WalkStop);
    }

    void walk_enter(const LegsSM::Ctx& c) { play(c.scene.get<Sprite>(c.entity), Legs::Walk); }

    void air_enter(const LegsSM::Ctx& c)
    {
        const Player::Intent in = c.scene.get<Player::Intent>(c.entity);

        // side jump only when the jump starts while moving, not for falling or steering later
        const bool side = (in.just_pressed & Player::Intent::Jump) && in.dir.x != 0;
        play(c.scene.get<Sprite>(c.entity), side ? Legs::SideJump : Legs::InAir);

        // the turns and the crouch transitions are full body sprites standing on the ground, drop them.
        // raising / lowering the gun is only the torso and carries on
        const entity_id body = body_of(c);
        const Pose      t    = c.scene.get<BodyData>(body).transition;

        if (body_state(c) == BodyState::Transition && t != Pose::LookUp && t != Pose::LookUpEnd)
            body_sm.force(c.scene, body, BodyState::Ready);
    }

    // the crouch sprites are full body, late_update hides the legs for those
    void crouch_enter(const LegsSM::Ctx& c) { begin_transition(c.scene, body_of(c), Pose::CrouchDown); }

    // also when leaving for Air, air_enter runs right after and drops it
    void crouch_exit(const LegsSM::Ctx& c) { begin_transition(c.scene, body_of(c), Pose::CrouchUp); }


    // what Stand, Walk and Crouch share
    LegsState grounded(const LegsSM::Ctx& c, const float speed, const bool can_jump = true)
    {
        const Player::Intent in  = c.scene.get<Player::Intent>(c.entity);
        Collider&            col = c.scene.get<Collider>(c.entity);

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
        if (can_jump && (in.just_pressed & Player::Intent::Jump))
        {
            col.velocity.y = -JUMP_VELOCITY;
            return LegsState::Air;
        }

        if (in.dir.y > 0) return LegsState::Crouch;
        return in.dir.x != 0 ? LegsState::Walk : LegsState::Stand;
    }

    LegsState stand(const LegsSM::Ctx& c) { return grounded(c, 0.0f); }
    LegsState walk(const LegsSM::Ctx& c) { return grounded(c, WALK_SPEED); }

    LegsState crouch(const LegsSM::Ctx& c)
    {
        const Player::Intent in = c.scene.get<Player::Intent>(c.entity);

        // no crawling or jumping while shooting, only turning and standing up.
        const bool shooting = body_state(c) == BodyState::Shooting || (in.down & Player::Intent::Fire);

        return grounded(c, shooting ? 0.0f : CRAWL_SPEED, !shooting);
    }

    LegsState air(const LegsSM::Ctx& c)
    {
        const Player::Intent in  = c.scene.get<Player::Intent>(c.entity);
        Collider&            col = c.scene.get<Collider>(c.entity);

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

    const LegsSM legs_sm
    {
        LegsSM::Behaviour{ stand_enter, stand, nullptr },         // Stand
        LegsSM::Behaviour{ walk_enter, walk, nullptr },           // Walk
        LegsSM::Behaviour{ air_enter, air, nullptr },             // Air
        LegsSM::Behaviour{ crouch_enter, crouch, crouch_exit },   // Crouch
    };


    // center of a sprite (= its Transform) from the center of the legs' box and the sprite's origin
    ivec2 sprite_center(const ivec2 box_center, const ivec2 origin, const ivec2 size, const bool flip)
    {
        const ivec2 box_min = box_center - ivec2(LEGS_SIZE / 2u);
        const ivec2 box_max = box_min + ivec2(LEGS_SIZE);

        // mirrored around the box when facing left
        const int32_t left = flip ? box_max.x - origin.x - size.x : box_min.x + origin.x;

        return ivec2{ left, box_min.y + origin.y } + size / 2;
    }
}


void Player::register_machines()
{
    // legs must be first, the body reads their state
    StateMachineSystem& sms = App::ctx().state_machines;
    sms.add(legs_sm);
    sms.add(body_sm);
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

    legs_sm.start(scene, legs_, LegsState::Stand);
    body_sm.start(scene, body_, BodyState::Ready);
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
        body_spr.anim_done() && playing.end == End::Next)
    {
        play(body_spr, playing.next);
    }

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
}