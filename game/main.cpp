#include "mse/game.hpp"

#include "layer.hpp"
#include "player.hpp"


using namespace mse;
using namespace mse::literals;

using anim::Enemy, anim::Bg;

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

        alignas(T) uint8_t data[sizeof(T)];
    };

    Maybe<Player> player;
    entity_id bg;

    void create_bg(Scene& scene)
    {
        scene.bg_entity = bg = scene.create_entity();

        Transform& tr = scene.set<Transform>(bg);
        Sprite& sprite = scene.set<Sprite>(bg, Bg::Mission1);

        sprite.layer = -100;
        tr.position.x += (int32_t)sprite.size().x / 2;
        tr.position.y += ((int32_t)sprite.size().y - RESOLUTION_i.y) / 2;
    }
}

void Game::init()
{
    Player::register_machines();

    Scene& scene = ctx().scene;

    create_bg(scene);
    player.set(scene);
}

void Game::update()
{
    player->update(ctx().scene);
}

void Game::late_update()
{
    player->late_update(ctx().scene);
}