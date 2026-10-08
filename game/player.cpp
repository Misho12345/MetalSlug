#include "mse/mse.hpp"
#include "player.hpp"
#include "layer.hpp"

using namespace mse;
using namespace mse::anim::player;

using LegsSM = StateMachine<LegsState>;
using BodySM = StateMachine<BodyState>;

namespace
{
    constexpr float JUMP_HEIGHT    = 56.0f;
    constexpr float JUMP_APEX_TIME = 0.36f; // from takeoff to the highest point

    constexpr float JUMP_GRAVITY  = 2.0f * JUMP_HEIGHT / (JUMP_APEX_TIME * JUMP_APEX_TIME);
    constexpr float JUMP_VELOCITY = 2.0f * JUMP_HEIGHT / JUMP_APEX_TIME;

    constexpr float WALK_SPEED = 100.0f;

    constexpr uint16_t TIRED_AFTER = 300; // ticks

    void face(const LegsSM::Ctx& c, const int8_t dir)
    {
        if (dir != 0) c.scene.get<SpriteRenderer>(c.entity).flip_x = dir < 0;
    }

    bool try_jump(SpriteCollider& col, const Intent in)
    {
        if (!(in.bits & Intent::Jump)) return false;
        col.velocity.y = -JUMP_VELOCITY;
        return true;
    }


    // ----- Legs -----

    void stand_enter(const LegsSM::Ctx& c) { c.scene.get<SpriteRenderer>(c.entity).play(Legs::Still); }
    void walk_enter (const LegsSM::Ctx& c) { c.scene.get<SpriteRenderer>(c.entity).play(Legs::Idle); }  // TODO: walk anim
    void air_enter  (const LegsSM::Ctx& c) { c.scene.get<SpriteRenderer>(c.entity).play(Legs::Still); } // TODO: jump anim

    LegsState stand(const LegsSM::Ctx& ctx)
    {
        const Intent    in  = ctx.scene.get<Intent>(ctx.entity);
        SpriteCollider& col = ctx.scene.get<SpriteCollider>(ctx.entity);

        col.velocity.x = 0.0f;

        if (!col.grounded) return LegsState::Air;
        if (try_jump(col, in)) return LegsState::Air;
        if (in.dir != 0) return LegsState::Walk;

        return LegsState::Stand;
    }

    LegsState walk(const LegsSM::Ctx& c)
    {
        const Intent    in  = c.scene.get<Intent>(c.entity);
        SpriteCollider& col = c.scene.get<SpriteCollider>(c.entity);

        col.velocity.x = (float)in.dir * WALK_SPEED;
        face(c, in.dir);

        if (!col.grounded) return LegsState::Air;
        if (try_jump(col, in)) return LegsState::Air;
        if (in.dir == 0) return LegsState::Stand;

        return LegsState::Walk;
    }

    LegsState air(const LegsSM::Ctx& c)
    {
        const Intent    in  = c.scene.get<Intent>(c.entity);
        SpriteCollider& col = c.scene.get<SpriteCollider>(c.entity);

        col.velocity.x = (float)in.dir * WALK_SPEED;
        face(c, in.dir);
        if (col.grounded) return in.dir != 0 ? LegsState::Walk : LegsState::Stand;

        return LegsState::Air;
    }

    const LegsSM legs_sm
    {
        LegsSM::Behaviour{ stand_enter, stand, nullptr }, // Stand
        LegsSM::Behaviour{ walk_enter,  walk,  nullptr }, // Walk
        LegsSM::Behaviour{ air_enter,   air,   nullptr }, // Air
    };



    // ---- Body ------

    entity_id legs_of(const BodySM::Ctx& c) { return c.scene.get<AttachedTo>(c.entity).entity; }

    void idle_enter    (const BodySM::Ctx& c) { c.scene.get<SpriteRenderer>(c.entity).play(Body::Idle); }
    void tired_enter   (const BodySM::Ctx& c) { c.scene.get<SpriteRenderer>(c.entity).play(Body::Tired); }
    void drinking_enter(const BodySM::Ctx& c) { c.scene.get<SpriteRenderer>(c.entity).play(Body::Drinking); }

    BodyState idle(const BodySM::Ctx& c)
    {
        const entity_id l  = legs_of(c);
        const Intent    in = c.scene.get<Intent>(l);

        const State<LegsState>& legs = c.scene.get<State<LegsState>>(l);

        if (in.bits & Intent::Drink) return BodyState::Drinking;

        if (legs.current == LegsState::Stand &&
            legs.ticks >= TIRED_AFTER && c.state.ticks >= TIRED_AFTER)
            return BodyState::Tired;

        return BodyState::Idle;
    }

    BodyState tired(const BodySM::Ctx& c)
    {
        const entity_id l  = legs_of(c);
        const Intent    in = c.scene.get<Intent>(l);

        const State<LegsState>& legs = c.scene.get<State<LegsState>>(l);

        if (in.bits & Intent::Drink) return BodyState::Drinking;
        if (legs.current != LegsState::Stand) return BodyState::Idle;
        return BodyState::Tired;
    }


    BodyState drinking(const BodySM::Ctx& c)
    {
        const Intent in = c.scene.get<Intent>(legs_of(c));
        return (in.bits & Intent::Drink) ? BodyState::Drinking : BodyState::Tired;
    }


    const BodySM body_sm
    {
        BodySM::Behaviour{ idle_enter,     idle,     nullptr }, // Idle
        BodySM::Behaviour{ tired_enter,    tired,    nullptr }, // Tired
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
    player_ = scene.create_entity();
    body_ = scene.create_entity();

    scene.set<Transform>(player_, ivec2{ 100, -50 });
    scene.set<Transform>(body_);

    scene.set<SpriteRenderer>(player_, Legs::Idle);
    scene.set<SpriteRenderer>(body_, Body::Idle);

    SpriteCollider& legs_sc = scene.set<SpriteCollider>(player_);
    legs_sc.layer = (uint32_t)Layer::Player;
    legs_sc.gravity = JUMP_GRAVITY;

    SpriteCollider& body_sc = scene.set<SpriteCollider>(body_);
    body_sc.layer = (uint32_t)Layer::Player;

    body_sc.gravity = 0.0f;
    body_sc.collide_with_tile_map = false;

    scene.set<Intent>(player_);
    scene.set<AttachedTo>(body_, player_);

    legs_sm.start(scene, player_, LegsState::Stand);
    body_sm.start(scene, body_, BodyState::Idle);
}


void Player::update(Scene& scene) const
{
    Intent& in = scene.set<Intent>(player_); // resets the intent

    if (Input::down(Key::A)) --in.dir;
    if (Input::down(Key::D)) ++in.dir;

    if (Input::just_pressed(Key::W)) in.bits |= Intent::Jump;
    if (Input::down(Key::Space)) in.bits |= Intent::Drink;
}

void Player::late_update(Scene& scene) const
{
    scene.get<SpriteRenderer>(body_).flip_x = scene.get<SpriteRenderer>(player_).flip_x;

    const Transform&      legs_tr = scene.get<Transform>(player_);
    const SpriteRenderer& legs_sr = scene.get<SpriteRenderer>(player_);

    Transform&      body_tr = scene.get<Transform>(body_);
    SpriteRenderer& body_sr = scene.get<SpriteRenderer>(body_);

    body_sr.flip_x = legs_sr.flip_x;

    scene.get<Transform>(scene.camera()).position = body_tr.position =
            legs_tr.position - ivec2{
                0, (legs_sr.size().y * legs_tr.scale.y +
                    body_sr.size().y * body_tr.scale.y) / 2
            };
}
