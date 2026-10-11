#include "mse/mse.hpp"
#include "player.hpp"
#include "layer.hpp"

using namespace mse;
using namespace mse::anim::player;

using LegsSM = StateMachine<LegsState>;
using BodySM = StateMachine<BodyState>;

namespace
{
    // ----- Movement constants -----

    constexpr float JUMP_HEIGHT    = 56.0f;
    constexpr float JUMP_APEX_TIME = 0.36f; // from takeoff to the highest point

    constexpr float JUMP_GRAVITY  = 2.0f * JUMP_HEIGHT / (JUMP_APEX_TIME * JUMP_APEX_TIME);
    constexpr float JUMP_VELOCITY = 2.0f * JUMP_HEIGHT / JUMP_APEX_TIME;

    constexpr float WALK_SPEED = 100.0f;
    constexpr float CRAWL_SPEED = 50.0f;


    // ----- Animation constants -----

    namespace dur
    {
        template <Legs V>
        using frames = const uint8_t[anim::frame_count(anim::info(V))];

        constexpr frames<Legs::SideJump> JUMP_FRAMES{ 4, 8, 12, 12, 8, 4 };


        template <Legs V> requires (V != Legs::SideJump)
        constexpr uint8_t get()
        {
            switch (V)
            {
                case Legs::Idle: return 0;
                case Legs::InAir: return 4;
                case Legs::Walk: return 4;
                default: return 0;
            }
        }

        template <Legs V> requires (V == Legs::SideJump)
        constexpr frames<V>& get()
        {
            return JUMP_FRAMES;
        }


        template <Body V>
        constexpr uint8_t get()
        {
            switch (V)
            {
                case Body::CrouchPistolShoot: return 0;
                case Body::CrouchPistolAfterShoot: return 0;

                // case Body::CrouchPistolKnifeSlash: return 0;
                // case Body::CrouchPistolKnifeStab: return 0;

                // case Body::CrouchPistolThrow: return 0;
                // case Body::CrouchPistolAfterThrow: return 0;

                case Body::CrouchPistol: return 0;
                case Body::CrouchPistolIdle: return 0;
                case Body::CrouchPistolWalk: return 0;
                case Body::CrouchPistolTurn: return 0;


                case Body::PistolIdle: return 0;

                case Body::PistolWalk: return 0;
                case Body::PistolWalkStop: return 0;
                case Body::PistolTurn: return 0;

                case Body::PistolInAir: return 0;

                case Body::PistolShoot: return 0;
                case Body::PistolSideJump: return 0;
                case Body::PistolJumpShoot: return 0;

                // case Body::PistolKnifeSlash: return 0;
                // case Body::PistolKnifeStab: return 0;

                // only in air
                case Body::PistolLookDown: return 0;
                case Body::PistolShootDown: return 0;

                case Body::PistolLookUp: return 0;
                case Body::PistolLookUpIdle: return 0;
                case Body::PistolLookUpTurn: return 0;
                case Body::PistolShootUp: return 0;

                // case Body::PistolReload: return 0;
                // case Body::PistolDrink: return 0;
                // case Body::PistolThrow: return 0;
                default: return 0;
            }
        }
    }


    // ----- Collider constants -----




    // util
    void face(const LegsSM::Ctx& c, const int8_t dir) { if (dir != 0) c.scene.get<Sprite>(c.entity).flip_x = dir < 0; }

    bool try_jump(Collider& col, const Player::Intent in)
    {
        if (!(in.bits & Player::Intent::Jump)) return false;
        col.velocity.y = -JUMP_VELOCITY;
        return true;
    }


    // ----- Legs -----

    void stand_enter(const LegsSM::Ctx& c) { c.scene.get<Sprite>(c.entity).play(Legs::Idle); }
    void walk_enter(const LegsSM::Ctx& c) { c.scene.get<Sprite>(c.entity).play(Legs::Walk, dur::get<Legs::Walk>()); }
    void air_enter(const LegsSM::Ctx& c) { c.scene.get<Sprite>(c.entity).play(Legs::InAir, dur::get<Legs::InAir>()); }
    void crouch_enter(const LegsSM::Ctx& c) { c.scene.get<Sprite>(c.entity).hidden = true; }

    LegsState stand(const LegsSM::Ctx& ctx)
    {
        const Player::Intent in = ctx.scene.get<Player::Intent>(ctx.entity);
        Collider& col = ctx.scene.get<Collider>(ctx.entity);

        col.velocity.x = 0.0f;

        if (!col.grounded) return LegsState::Air;
        if (try_jump(col, in)) return LegsState::Air;
        if (in.dir.x != 0) return LegsState::Walk;

        return LegsState::Idle;
    }

    LegsState walk(const LegsSM::Ctx& c)
    {
        const Player::Intent in  = c.scene.get<Player::Intent>(c.entity);
        Collider&            col = c.scene.get<Collider>(c.entity);

        col.velocity.x = (float)in.dir * WALK_SPEED;
        face(c, in.dir);

        if (!col.grounded) return LegsState::Air;
        if (try_jump(col, in)) return LegsState::Air;
        if (in.dir == 0) return LegsState::Idle;

        return LegsState::Walk;
    }

    LegsState air(const LegsSM::Ctx& c)
    {
        const Player::Intent in  = c.scene.get<Player::Intent>(c.entity);
        Collider&            col = c.scene.get<Collider>(c.entity);

        col.velocity.x = (float)in.dir * WALK_SPEED;
        face(c, in.dir);
        if (col.grounded) return in.dir != 0 ? LegsState::Walk : LegsState::Stand;

        return LegsState::Air;
    }

    LegsState crouch(const LegsSM::Ctx& c) {}

    const LegsSM legs_sm
    {
        LegsSM::Behaviour{ stand_enter, stand, nullptr }, // Stand
        LegsSM::Behaviour{ walk_enter, walk, nullptr },   // Walk
        LegsSM::Behaviour{ air_enter, air, nullptr },     // Air
        LegsSM::Behaviour{ crouch_enter, crouch, nullptr },     // Crouch
    };


    // ----- Body -----

    entity_id legs_of(const BodySM::Ctx& c) { return c.scene.get<Player::AttachedTo>(c.entity).entity; }

    void idle_enter(const BodySM::Ctx& c) { c.scene.get<Sprite>(c.entity).play(Body::PistolIdle); }
    void tired_enter(const BodySM::Ctx& c) { c.scene.get<Sprite>(c.entity).play(Body::PistolLookDown); }
    void drinking_enter(const BodySM::Ctx& c) { c.scene.get<Sprite>(c.entity).play(Body::PistolDrink); }

    BodyState idle(const BodySM::Ctx& c)
    {
        const entity_id      l  = legs_of(c);
        const Player::Intent in = c.scene.get<Player::Intent>(l);

        const State<LegsState>& legs = c.scene.get<State<LegsState>>(l);

        if (in.bits & Player::Intent::Drink) return BodyState::Drinking;

        if (legs.current == LegsState::Stand &&
            legs.ticks >= TIRED_AFTER && c.state.ticks >= TIRED_AFTER)
            return BodyState::Tired;

        return BodyState::Idle;
    }

    BodyState tired(const BodySM::Ctx& c)
    {
        const entity_id      l  = legs_of(c);
        const Player::Intent in = c.scene.get<Player::Intent>(l);

        const State<LegsState>& legs = c.scene.get<State<LegsState>>(l);

        if (in.bits & Player::Intent::Drink) return BodyState::Drinking;
        if (legs.current != LegsState::Stand) return BodyState::Idle;
        return BodyState::Tired;
    }


    BodyState drinking(const BodySM::Ctx& c)
    {
        const Player::Intent in = c.scene.get<Player::Intent>(legs_of(c));
        return (in.bits & Player::Intent::Drink) ? BodyState::Drinking : BodyState::Tired;
    }


    const BodySM body_sm
    {
        BodySM::Behaviour{ idle_enter, idle, nullptr },         // Idle
        BodySM::Behaviour{ tired_enter, tired, nullptr },       // Tired
        BodySM::Behaviour{ drinking_enter, drinking, nullptr }, // Drinking
    };
}


void Player::register_machines()
{
    StateMachineSystem& sms = App::ctx().state_machines;
    sms.add(legs_sm);
    sms.add(body_sm);
}


Player::Player(Scene& scene)
{
    legs_ = scene.create_entity();
    body_   = scene.create_entity();

    scene.set<Transform>(legs_, ivec2{ 100, -50 });
    scene.set<Transform>(body_);

    scene.set<Sprite>(legs_, Legs::Idle);
    scene.set<Sprite>(body_, Body::PistolIdle);

    Collider& legs_sc = scene.set<Collider>(legs_);
    legs_sc.layer     = (uint32_t)Layer::Player;
    legs_sc.gravity   = JUMP_GRAVITY;

    Collider& body_sc = scene.set<Collider>(body_);
    body_sc.layer     = (uint32_t)Layer::Player;

    body_sc.gravity               = 0.0f;
    body_sc.collide_with_tile_map = false;

    scene.set<Intent>(legs_);
    scene.set<AttachedTo>(body_, legs_);

    legs_sm.start(scene, legs_, LegsState::Stand);
    body_sm.start(scene, body_, BodyState::Idle);
}


void Player::update(Scene& scene) const
{
    Intent& in = scene.set<Intent>(legs_); // resets the intent

    if (Input::down(Key::A)) --in.dir;
    if (Input::down(Key::D)) ++in.dir;

    if (Input::just_pressed(Key::W)) in.bits |= Intent::Jump;
    if (Input::down(Key::Space)) in.bits |= Intent::Drink;
}

void Player::late_update(Scene& scene) const
{
    scene.get<Sprite>(body_).flip_x = scene.get<Sprite>(legs_).flip_x;

    const Transform& legs_tr = scene.get<Transform>(legs_);
    const Sprite&    legs_sr = scene.get<Sprite>(legs_);

    Transform& body_tr = scene.get<Transform>(body_);
    Sprite&    body_sr = scene.get<Sprite>(body_);

    body_sr.flip_x = legs_sr.flip_x;

    scene.get<Transform>(scene.camera()).position = body_tr.position =
            legs_tr.position - ivec2{
                0, (legs_sr.size().y * legs_tr.scale.y +
                    body_sr.size().y * body_tr.scale.y) / 2
            };
}
