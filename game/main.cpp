#include <thread>

#include "mse/game.hpp"

using namespace mse;
using namespace mse::anim;
using namespace mse::literals;

namespace
{
    entity_id player_legs, player_body;
    entity_id slon;

    glm::ivec2 get_input()
    {
        const bool w = Input::down(Key::W);
        const bool a = Input::down(Key::A);
        const bool s = Input::down(Key::S);
        const bool d = Input::down(Key::D);

        return glm::ivec2{
            static_cast<int>(d) - static_cast<int>(a),
            static_cast<int>(s) - static_cast<int>(w)
        };
    }
}

void Game::init()
{
    Scene& scene = ctx().scene;

    player_legs = scene.create_entity();
    player_body = scene.create_entity();
    slon        = scene.create_entity();

    scene.set<Transform>(player_legs, glm::ivec2{ 150, 64 });
    scene.set<Transform>(player_body);
    scene.set<Transform>(slon, glm::ivec2{ 160, 150 });

    scene.set<SpriteRenderer>(player_legs, player::Legs::Idle);
    scene.set<SpriteRenderer>(player_body, player::Body::Idle);
    scene.set<SpriteRenderer>(slon, Enemy::Slon);

    scene.set<SpriteCollider>(player_legs, player_legs);
    scene.set<SpriteCollider>(player_body, player_body);
    scene.set<SpriteCollider>(slon, slon);
}

void Game::update(const float)
{
    Scene& s = ctx().scene;

    Transform& legs = s.get<Transform>(player_legs);
    Transform& body = s.get<Transform>(player_body);

    SpriteRenderer& legs_sr = s.get<SpriteRenderer>(player_legs);
    SpriteRenderer& body_sr = s.get<SpriteRenderer>(player_body);

    body.position = legs.position - glm::ivec2{ 0, (legs_sr.size().y * legs.scale.y + body_sr.size().y * body.scale.y) / 2 };

    if (Input::just_pressed(Key::Space)) body_sr.play(player::Body::Drinking);
    else if (Input::just_released(Key::Space)) body_sr.play(player::Body::Tired);

    if (Input::just_pressed(Key::Enter))
    {
        printf(
            "%f, %f\n",
            static_cast<double>(legs.position.x),
            static_cast<double>(legs.position.y));
    }

    if (Input::just_pressed(Key::Minus))
    {
        legs.scale = glm::max(legs.scale - 1, glm::ivec2{ 1 });
        body.scale = glm::max(body.scale - 1, glm::ivec2{ 1 });
    }

    if (Input::just_pressed(Key::Equal))
    {
        legs.scale = glm::max(legs.scale + 1, glm::ivec2{ 1 });
        body.scale = glm::max(body.scale + 1, glm::ivec2{ 1 });
    }

    // temporary, will implement a cross platform sleep later
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
}

void Game::fixed_update()
{
    Scene& s = ctx().scene;
    const glm::vec2 input = get_input();

    if (input.x != 0.0f || input.y != 0.0f)
    {
        s.get<SpriteCollider>(player_legs).apply_force(input * 100.0f);
    }
}
