#include "layer.hpp"
#include "mse/game.hpp"

#include "player.hpp"


using namespace mse;
using namespace mse::literals;

using anim::Enemy, anim::Bg;
using player::Player;

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

    Maybe<Player> player_m;
    entity_id bg;

    void create_bg(Scene& scene)
    {
        scene.bg_entity = bg = scene.create_entity();

        Transform& tr = scene.set<Transform>(bg);
        Sprite& sprite = scene.set<Sprite>(bg, Bg::Bg, should_loop::no);

        sprite.layer = -100;
        tr.position.x += (int32_t)sprite.size().x / 2;
        tr.position.y += ((int32_t)sprite.size().y - RESOLUTION_i.y) / 2;
    }

    struct POWData { bool sitting; }; // temp

    void spawn_pow(Scene& scene, const ivec2 pos, const bool sitting)
    {
        const entity_id pow = scene.create_entity();

        scene.set<Transform>(pow, pos);
        scene.set<Sprite>(pow, sitting ? anim::Pow::SittingIdle : anim::Pow::TreeIdle, should_loop::yes);
        scene.set<POWData>(pow, sitting);

        Collider& col = scene.set<Collider>(pow);
        col.target_layer |= Layer::PlayerBullet;

        col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
        {
            Sprite& sprite = scene.get<Sprite>(self);
            sprite.play(scene.get<POWData>(self).sitting ? anim::Pow::SittingFree : anim::Pow::TreeFree);
            scene.get<Collider>(self).callback = nullptr;

            sprite.color = 0xff0000_rgb;
        };
    }

    void load_pows(Scene& scene)
    {
        const Csv csv = FileIO::read<Csv>("assets/tile_maps/mission1_spawn.csv");
        const int32_t* d = csv.data.data();

        const ivec2 off = scene.get<Sprite>(scene.bg_entity).bounds(scene.get<Transform>(scene.bg_entity)).min;

        for (uint32_t y = 0; y < csv.height; ++y)
        {
            for (uint32_t x = 0; x < csv.width; ++x, ++d)
            {
                const ivec2 pos = ivec2{ x, (int32_t)y - 1 } * TILE_SIZE_i + off;

                switch (*d)
                {
                    case -1: break;
                    case 0: spawn_pow(scene, pos, true); break;
                    case 1: spawn_pow(scene, pos, false); break;
                    // case 2: enemy 1;
                    // case 3: enemy 2;
                    default: assert(!"not implemented"); return;
                }
            }
        }
    }
}

void Game::init()
{
    Player::register_machines();
    load_tile_map("assets/tile_maps/mission1_collision.csv");

    Scene& scene = ctx().scene;

    create_bg(scene);
    player_m.set(scene);
    load_pows(scene);
}

void Game::update()
{
    player_m->update(ctx().scene);
}

void Game::late_update()
{
    player_m->late_update(ctx().scene);
}