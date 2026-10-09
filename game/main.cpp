#include "mse/game.hpp"

#include "layer.hpp"
#include "player.hpp"


using mse::Scene, mse::Transform, mse::SpriteRenderer, mse::SpriteCollider;
using mse::anim::Enemy, mse::anim::Bg;
using mse::entity_id;

using namespace mse::literals;

namespace
{
    template <typename T>
    struct Maybe
    {
        template <typename... Args>
        void set(Args&&... args) { new (data) T(std::forward<Args>(args)...); }
        void reset() { mse::destroy_at((T*)data); }

        T& operator*() { return *reinterpret_cast<T*>(data); }
        const T& operator*() const { return *reinterpret_cast<const T*>(data); }

        T* operator->() { return reinterpret_cast<T*>(data); }
        const T* operator->() const { return reinterpret_cast<const T*>(data); }

        uint8_t data[sizeof(T)];
    };

    entity_id slon;
    entity_id bg;

    Maybe<Player> player;
}

void Game::init()
{
    Player::register_machines();

    Scene& scene = ctx().scene;

    player.set(scene);
    slon = scene.create_entity();
    bg = scene.create_entity();

    scene.bg_entity = bg;

    // slon
    scene.set<Transform>(slon, mse::ivec2{ 100, 100 });
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
    bg_tr.position.y += ((int32_t)bg_sr.size().y - mse::RESOLUTION_i.y) / 2;
}

void Game::update()
{
    player->update(ctx().scene);
}

void Game::late_update()
{
    player->late_update(ctx().scene);
}