#include "layer.hpp"
#include "mse/game.hpp"

#include "player/player.hpp"
#include "pow.hpp"

using namespace mse;
using namespace mse::literals;

using anim::Bg;
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

    void place_rock(Scene& scene, const ivec2 pos, const uvec2 coords)
    {
        static constexpr uvec2 ROCK_SIZE{ 7, 6 };
        static constexpr ivec2 rock_off = ivec2(ROCK_SIZE) * TILE_SIZE_i / 2;

        const entity_id rock = scene.create_entity();
        scene.set<Transform>(rock, pos + rock_off); // tile is at top left of the rock
        scene.set<Sprite>(rock, anim::Other::Rock, should_loop::no).layer = -99;

        TileMap& tm = App::ctx().tile_map;

        const uvec2 end_coords = coords + ROCK_SIZE;

        for (uint32_t y = coords.y; y < end_coords.y; ++y)
        {
            for (uint32_t x = coords.x; x < end_coords.x; ++x)
                tm[{ x, y }] = TileType::Floor;
        }
    }

    void load_spawables(Scene& scene)
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
                    case 4: place_rock(scene, pos, { x, y }); break;
                    case 5: break;
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

    if (!ctx().tile_map.load("assets/tile_maps/mission1_collision.csv"))
    {
        printf("failed to load collision tile data");
        return;
    }

    Scene& scene = ctx().scene;

    create_bg(scene);
    player_opt = Player{ scene };
    load_spawables(scene);
}

void Game::update()
{
    player_opt->update(ctx().scene);
}

void Game::late_update()
{
    player_opt->late_update(ctx().scene);
}