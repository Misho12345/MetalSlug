#include <thread>

#include "../tools/texture_packer/atlas.hpp"
#include "mse/game.hpp"
#include "mse/input.hpp"

using namespace mse;
using namespace mse::anim;
using namespace mse::literals;

namespace
{
    entity_id player_legs, player_body;
    entity_id ground;

    glm::vec2 get_input()
    {
        const bool w = Input::down(Key::W);
        const bool a = Input::down(Key::A);
        const bool s = Input::down(Key::S);
        const bool d = Input::down(Key::D);

        return w - s || a - d
                   ? glm::normalize(glm::vec2{
                       static_cast<float>(d) - static_cast<float>(a),
                       static_cast<float>(s) - static_cast<float>(w)
                   })
                   : glm::vec2{ 0.0f, 0.0f };
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
        glm::vec2{ 300, 300 },
        glm::vec2{ 300, 300 }
    );

    scene.set<Transform>(
        player_body,
        legs_tr.position - 300.0f,
        glm::vec2{ 300, 300 }
    );

    Transform& gr_tr = scene.set<Transform>(
        ground,
        glm::vec2{ 600, 750 },
        glm::vec2{ 1200, 100 }
    );

    scene.set<SpriteRenderer>(player_legs, player::Legs::Idle);
    scene.set<SpriteRenderer>(player_body, player::Body::Idle);
    scene.set<SpriteRenderer>(ground, Enemy::Slon);

    scene.set<Rigidbody>(player_legs).gravity = false;
    scene.set<BoxCollider>(player_legs, glm::vec2{ 0, -300 }, glm::vec2{ 300, 600 });
    scene.set<BoxCollider>(ground, glm::vec2{}, gr_tr.scale);
}

void Game::update(const float)
{
    Scene& s = ctx().scene;

    SpriteRenderer& body_sr = s.get<SpriteRenderer>(player_body);

    if (Input::just_pressed(Key::Space)) body_sr.play(player::Body::Drinking);
    else if (Input::just_released(Key::Space)) body_sr.play(player::Body::Tired);

    s.get<Transform>(player_body).position =
            s.get<Transform>(player_legs).position -
            glm::vec2{ 0, s.get<Transform>(player_legs).scale.y / 2.0f };

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
        s.get<Rigidbody>(player_legs).apply_force(input * 5e2f);
    }
}