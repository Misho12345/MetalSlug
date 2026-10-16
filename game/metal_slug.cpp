#include "mse/mse.hpp"
#include "metal_slug.hpp"

#include "layer.hpp"

#include "player/player.hpp"
#include "player/components.hpp"

#include "pow.hpp"

using namespace mse;
using namespace mse::literals;

using anim::Bg;
using player::Player, player::WeaponUpgrade, player::WeaponType;


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

    void create_rock(Scene& scene, const ivec2 pos, const uvec2 coords)
    {
        static constexpr uvec2 ROCK_SIZE{ 7, 6 }; // in tiles
        static constexpr ivec2 rock_off = ivec2(ROCK_SIZE) * TILE_SIZE_i / 2;

        const entity_id rock = scene.create_entity();
        scene.set<Transform>(rock, pos + rock_off); // tile is at top left of the rock
        scene.set<Sprite>(rock, anim::Other::Rock, should_loop::no).layer = -99;

        TileMap& tm = App::ctx().tile_map;
        TileType* t = &tm[coords];

        for (uint32_t y = 0; y < ROCK_SIZE.y; ++y, t += tm.size().x)
            memset(t, (int)TileType::Floor, ROCK_SIZE.x);
    }


    void create_upgrade(Scene& scene, const ivec2 pos, WeaponType type)
    {

        static constexpr uvec2 CRATE_SIZE{ 2, 2 }; // in tiles
        static constexpr ivec2 crate_off = ivec2(CRATE_SIZE) * TILE_SIZE_i / 2;

        const entity_id upgrade = scene.create_entity();
        scene.set<WeaponUpgrade>(upgrade, type);
        scene.set<Transform>(upgrade, pos + crate_off);
        scene.set<Sprite>(upgrade, [type]() -> anim::Other
        {
            switch (type)
            {
                case WeaponType::RifleSpray: return anim::Other::RifleSprayPickup;
                case WeaponType::RifleFireball: return anim::Other::RifleFireballPickup;
                default: assert(!"not implemented");
            }

            return {};
        }(), should_loop::no);


        Collider& col = scene.set<Collider>(upgrade);

        col.gravity = 0.0f;
        col.collide_with_tile_map = false;

        col.layer |= Layer::WeaponUpgrade;
        col.target_layer |= Layer::Player;

        col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
        {
            scene.get<Collider>(self).enabled = false;
            scene.get<Sprite>(self).hidden = true;
        };
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
                    case 4: create_rock(scene, pos, { x, y }); break;
                    case 5:
                        // TODO: replace with crate
                        create_upgrade(scene, pos, WeaponType::RifleSpray);
                        create_upgrade(scene, pos - 20, WeaponType::RifleFireball);
                        create_upgrade(scene, pos - 40, WeaponType::RifleSpray);
                        break;
                    default: assert(!"not implemented"); return;
                }
            }
        }
    }
}

void MetalSlug::init()
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

void MetalSlug::update()
{
    player_opt->update(ctx().scene);
}

void MetalSlug::late_update()
{
    player_opt->late_update(ctx().scene);
}