#include "layer.hpp"
#include "mse/game.hpp"

#include "player.hpp"
#include "pow.hpp"

using namespace mse;
using namespace mse::literals;

using anim::Enemy, anim::Bg;
using player::Player;

namespace
{
    optional<Player> player_opt;
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
                    case 0: POW::spawn(scene, pos, POW::Kind::Sitting); break;
                    case 1: POW::spawn(scene, pos, POW::Kind::Tree); break;
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
    POW::register_machine();

    load_tile_map("assets/tile_maps/mission1_collision.csv");

    Scene& scene = ctx().scene;

    create_bg(scene);
    player_opt = Player{ scene };
    load_pows(scene);
}

void Game::update()
{
    player_opt->update(ctx().scene);
}

void Game::late_update()
{
    player_opt->late_update(ctx().scene);
}