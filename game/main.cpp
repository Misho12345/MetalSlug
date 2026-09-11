#include <thread>

#include "mse/game.hpp"
#include "mse/input.hpp"
#include "mse/sprite.hpp"

mse::Sprite* p;

using namespace mse::anim;
using mse::Sprite, mse::Input, mse::Key;

void Game::init()
{
    Sprite::set_frame_count(player::Body::BW0tmsz, 4);

    Sprite::set_frame_count(player::Legs::Still, 1);
    Sprite::set_frame_count(player::Legs::Idle, 4);

    Sprite::set_frame_count(Enemy::Mishki, 3);
    Sprite::set_frame_count(Enemy::Mishok, 3);
    Sprite::set_frame_count(Enemy::Slon, 3);

    p = &Sprite::create<player::Legs>({
        .pos  = { 300, 300 },
        .size = { 300, 300 },
        .rot  = 0.0f
    });

    p->play(player::Legs::Still);
    p->frame_dur = 0.2f;
}

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

void Game::tick(const float dt)
{
    const glm::vec2 input = get_input();

    if (input.x != 0.0f || input.y != 0.0f)
    {
        p->set_pos(p->pos() + get_input() * (dt * 1000.0f));
    }

    if (Input::just_pressed(Key::Space)) p->play(player::Legs::Idle);
    else if (Input::just_released(Key::Space)) p->play(player::Legs::Still);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
}
