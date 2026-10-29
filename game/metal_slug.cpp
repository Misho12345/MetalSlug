#include "mse/mse.hpp"
#include "metal_slug.hpp"

#include "user_ctx.hpp"

#include "props.hpp"

#include "player/components.hpp"
#include "hud/number.hpp"

using namespace mse;
using namespace mse::literals;

using anim::Bg;

namespace ms
{
    namespace
    {
        entity_id bg;

        void create_bg(Scene& scene)
        {
            scene.bg_entity = bg = scene.create_entity();

            Transform& tr = scene.set<Transform>(bg);
            Sprite& sprite = scene.set<Sprite>(bg, Bg::Bg);

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
            scene.set<Sprite>(rock, anim::Other::Rock).layer = -99;

            TileMap& tm = App::ctx().tile_map;
            TileType* t = &tm[coords];

            for (uint32_t y = 0; y < ROCK_SIZE.y; ++y, t += tm.size().x)
                memset(t, (int)TileType::Floor, ROCK_SIZE.x);
        }


        void load_spawables(Scene& scene)
        {
            const UserCtx& u = MetalSlug::user_ctx();

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
                        case 0: u.pows.spawn(scene, pos, POWManager::Kind::Sitting); break;
                        case 1: u.pows.spawn(scene, pos, POWManager::Kind::Tree); break;
                        case 2: u.enemies.spawn(scene, pos, EnemyKind::Rebel); break;
                        case 3: u.enemies.spawn(scene, pos, EnemyKind::AquaRebel); break;
                        case 4: create_rock(scene, pos, { x, y }); break;
                        case 5: u.pickups.spawn_crate(scene, pos); break;
                        default: assert(!"not implemented"); return;
                    }
                }
            }
        }

        // everything but the camera and the app entity. Done on entering the states that need an empty scene
        // and not on leaving gameplay, the debrief draws over the level
        void clear_scene(Scene& scene, const entity_id app)
        {
            for (const entity_id owner : scene.pool<Transform>().owners())
            {
                if (owner != scene.camera() && owner != app)
                    scene.destroy_entity(owner);
            }

            App::ctx().particles.clear();

            scene.set<Transform>(scene.camera()); // reset camera
            scene.backdrop = { 0, 0, 0 };          // the game over picture fades in from black
        }

        constexpr const char* INTRO_TEXT = "PRESS SPACE TO PLAY";
        constexpr ivec2       INTRO_TEXT_POS{ (RESOLUTION_i.x - 19 * 8) / 2, 190 };
        constexpr int32_t     INTRO_TEXT_LAYER = 101;
    }

    void MetalSlug::init()
    {
        Scene& scene = ctx().scene;
        StateMachineSystem& sms = ctx().state_machines;

        sms.add(sm);
        user_ctx_.player.register_machines();
        user_ctx_.pows.register_machine();
        user_ctx_.enemies.register_machines();
        user_ctx_.boss.register_machines();

        user_ctx_.sounds.load();

        app_ = scene.create_entity();
        scene.set<Transform>(app_);
        sm.start(scene, app_, GameState::Intro);
    }

    void MetalSlug::intro_enter(const SM::Ctx& ctx)
    {
        ctx.scene.set<Sprite>(ctx.entity, anim::Hud::Intro);

        user_ctx().sounds.stop_music();
        user_ctx().sounds.play(Sfx::Title);

        create_hud_text(ctx.scene, anim::Hud::MediumFont, INTRO_TEXT, INTRO_TEXT_POS, INTRO_TEXT_LAYER);
    }

    GameState MetalSlug::on_intro(const SM::Ctx&) { return Input::down(Key::Space) ? GameState::Gameplay : GameState::Intro; }

    void MetalSlug::intro_exit(const SM::Ctx& ctx)
    {
        // the text goes with gameplay_enter's clear
        ctx.scene.get<Sprite>(ctx.entity).hidden = true;
    }


    void MetalSlug::gameplay_enter(const SM::Ctx& ctx)
    {
        clear_scene(ctx.scene, ctx.entity);

        if (!App::ctx().tile_map.load("assets/tile_maps/mission1_collision.csv"))
        {
            printf("failed to load collision tile data\n");
            return;
        }

        create_bg(ctx.scene);
        load_props(ctx.scene);
        user_ctx().player.create(ctx.scene);
        load_spawables(ctx.scene);

        user_ctx().hud.create(ctx.scene);

        user_ctx().sounds.music(Music::Gameplay);
        user_ctx().sounds.play(Sfx::MissionStart);
    }


    void MetalSlug::debrief_enter(const SM::Ctx& ctx)
    {
        user_ctx().debrief.create(ctx.scene);
        user_ctx().sounds.music(Music::Debrief);
    }

    GameState MetalSlug::on_debrief(const SM::Ctx& ctx)
    {
        user_ctx().debrief.update(ctx.scene, ctx.state.ticks);
        return Debrief::done(ctx.state.ticks) ? GameState::GameOver : GameState::Debrief;
    }


    void MetalSlug::game_over_enter(const SM::Ctx& ctx)
    {
        clear_scene(ctx.scene, ctx.entity);

        Sprite& s = ctx.scene.get<Sprite>(ctx.entity);
        s.hidden = false;
        s.play(anim::Hud::GameOver);
        s.color = 0_rgb;

        user_ctx().sounds.music(Music::GameOver);
        user_ctx().sounds.play(Sfx::GameOver);
    }

    GameState MetalSlug::on_game_over(const SM::Ctx& ctx)
    {
        uint32_t c = TRANSITION_DUR - glm::abs(2 * ctx.state.ticks - (int)TRANSITION_DUR);
        c = c * 255 / TRANSITION_DUR;

        ctx.scene.get<Sprite>(ctx.entity).color.data = { c, c, c, 255 };
        // back to the title, like an arcade cabinet
        return ctx.state.ticks >= TRANSITION_DUR
                   ? GameState::Intro
                   : GameState::GameOver;
    }


    void MetalSlug::update()
    {
        Scene& scene = ctx().scene;
        const GameState state = scene.get<State<GameState>>(app_).current;

        if (state == GameState::Gameplay)
        {
            user_ctx_.player.update(scene);
        }
        else if (state == GameState::Debrief)
        {
            // the level keeps going under the debrief, but the player stops instead of
            // keeping whatever was held on the last gameplay tick
            scene.set<Intent>(user_ctx_.player.legs());
        }
    }

    void MetalSlug::late_update()
    {
        Scene& scene = ctx().scene;

        const GameState state = scene.get<State<GameState>>(app_).current;

        if (state == GameState::Gameplay)
        {
            user_ctx_.boss.late_update(scene); // before the player's sprites and the camera are placed
            user_ctx_.player.late_update(scene);
            user_ctx_.pickups.update(scene);
            update_props(scene);
            user_ctx_.hud.update(scene);

            if (Input::just_pressed(Key::Esc) || user_ctx_.player.out_of_lives(scene) || user_ctx_.boss.defeated(scene))
                sm.force(scene, app_, GameState::Debrief);
        }
        else if (state == GameState::Debrief)
        {
            user_ctx_.hud.update(scene); // for the bonus going into the score
        }

        if (Input::down(Key::LCtrl) && Input::just_pressed(Key::R))
        {
            sm.force(scene, app_, GameState::Gameplay, reset_state_if_same::yes);
        }
    }

    UserCtx& MetalSlug::user_ctx() { return ((MetalSlug*)instance_)->user_ctx_; }
}