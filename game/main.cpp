#include "mse/game.hpp"

using namespace mse;
using namespace mse::anim;
using namespace mse::literals;

namespace
{
    entity_id player_legs, player_body;
    entity_id slon;
    entity_id bg;

    enum class Layer
    {
        Player = 1 << 0,
        Enemy  = 1 << 1
    };

    constexpr float JUMP_HEIGHT    = 56.0f;
    constexpr float JUMP_APEX_TIME = 0.36f; // from takeoff to the highest point

    constexpr float JUMP_GRAVITY  = 2.0f * JUMP_HEIGHT / (JUMP_APEX_TIME * JUMP_APEX_TIME);
    constexpr float JUMP_VELOCITY = 2.0f * JUMP_HEIGHT / JUMP_APEX_TIME;
}


void Game::init()
{
    Scene& scene = ctx().scene;

    player_legs = scene.create_entity();
    player_body = scene.create_entity();
    slon = scene.create_entity();
    bg = scene.create_entity();

    scene.bg_entity = bg;

    // player and slon
    scene.set<Transform>(player_body);
    scene.set<Transform>(player_legs, ivec2{ 100, -50 });

    scene.set<SpriteRenderer>(player_body, player::Body::Idle);
    scene.set<SpriteRenderer>(player_legs, player::Legs::Idle);

    // the body is placed on the legs in late_update, so it shouldn't fall or collide with tiles
    SpriteCollider& body_sc = scene.set<SpriteCollider>(player_body);
    body_sc.layer                 = (uint32_t)Layer::Player;
    body_sc.gravity               = 0.0f;
    body_sc.collide_with_tile_map = false;

    SpriteCollider& legs_sc = scene.set<SpriteCollider>(player_legs);
    legs_sc.layer   = (uint32_t)Layer::Player;
    legs_sc.gravity = JUMP_GRAVITY;

    // slon
    scene.set<Transform>(slon, ivec2{ 100, 100 });
    scene.set<SpriteRenderer>(slon, Enemy::Slon);

    SpriteCollider& slon_sc = scene.set<SpriteCollider>(slon);

    slon_sc.layer = (uint32_t)Layer::Enemy;

    slon_sc.target_layer = (uint32_t)Layer::Player;
    slon_sc.callback = +[](const entity_id id, const uint32_t mask)
    {
        assert(mask & (uint32_t)Layer::Player);
        ctx().scene.get<Transform>(id).rotation += 3.0_deg;
    };

    // bg
    Transform& bg_tr = scene.set<Transform>(bg);
    SpriteRenderer& bg_sr = scene.set<SpriteRenderer>(bg, Bg::Mission1);

    bg_sr.layer = -100;
    bg_tr.position.x += (int32_t)bg_sr.size().x / 2;
    bg_tr.position.y += ((int32_t)bg_sr.size().y - Target::RESOLUTION.y) / 2;
}

void Game::update()
{
    Scene& s = ctx().scene;

    SpriteRenderer& body_sr = s.get<SpriteRenderer>(player_body);
    SpriteCollider& legs_cs = s.get<SpriteCollider>(player_legs);

    if (Input::just_pressed(Key::Enter))  body_sr.flip_x ^= 1;

    if (Input::just_pressed(Key::Space)) body_sr.play(player::Body::Drinking);
    else if (Input::just_released(Key::Space)) body_sr.play(player::Body::Tired);

    float dir{};
    if (Input::down(Key::A)) --dir;
    if (Input::down(Key::D)) ++dir;

    legs_cs.velocity.x = dir * 100.0f;

    if (legs_cs.grounded && Input::just_pressed(Key::W)) legs_cs.velocity.y = -JUMP_VELOCITY;
}

void Game::late_update()
{
    Scene& s = ctx().scene;
    Transform& cam = s.get<Transform>(s.camera());

    const Transform& legs = s.get<Transform>(player_legs);
    Transform& body = s.get<Transform>(player_body);

    const SpriteRenderer& legs_sr = s.get<SpriteRenderer>(player_legs);
    const SpriteRenderer& body_sr = s.get<SpriteRenderer>(player_body);

    cam.position = body.position = legs.position - ivec2{
        0, (
            legs_sr.size().y * legs.scale.y +
            body_sr.size().y * body.scale.y
        ) / 2
    };
}