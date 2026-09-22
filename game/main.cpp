#include <thread>

#include "mse/game.hpp"

using namespace mse;
using namespace mse::anim;
using namespace mse::literals;

namespace
{
    entity_id player_legs, player_body;
    entity_id ground;

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
    ground      = scene.create_entity();

    const Transform& legs_tr = scene.set<Transform>(
        player_legs,
        glm::ivec2{ Target::RESOLUTION.x / 2, 64 },
        glm::ivec2{ 32, 32 }
    );

    scene.set<Transform>(
        player_body,
        legs_tr.position - glm::ivec2{ 0, 32 },
        glm::ivec2{ 32, 32 }
    );

    scene.set<Transform>(
        ground,
        glm::ivec2{ Target::RESOLUTION.x / 2, Target::RESOLUTION.y - 16 },
        glm::ivec2{ Target::RESOLUTION.x, 16 }
    );

    scene.set<SpriteRenderer>(player_legs, player::Legs::Idle);
    scene.set<SpriteRenderer>(player_body, player::Body::Idle);
    scene.set<SpriteRenderer>(ground, Enemy::Slon);

    scene.set<SpriteCollider>(player_legs, glm::ivec2{ 0, -32 }, glm::ivec2{ 32, 64 });
    scene.set<SpriteCollider>(ground, ground);
}

void Game::update(const float)
{
    Scene& s = ctx().scene;

    SpriteRenderer& body_sr = s.get<SpriteRenderer>(player_body);

    if (Input::just_pressed(Key::Space)) body_sr.play(player::Body::Drinking);
    else if (Input::just_released(Key::Space)) body_sr.play(player::Body::Tired);

    s.get<Transform>(player_body).position =
            s.get<Transform>(player_legs).position -
            glm::ivec2{ 0, 32 };

    if (Input::just_pressed(Key::Enter))
    {
        Transform& tr = s.get<Transform>(player_legs);
        printf("%f, %f\n", static_cast<double>(tr.position.x), static_cast<double>(tr.position.y));
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