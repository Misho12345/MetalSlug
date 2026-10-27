#include "hud.hpp"
#include "metal_slug.hpp"
#include "player/components.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        // top left of the panel, in screen pixels
        constexpr ivec2 PANEL_POS{ 80, 7 };

        // from the panel's top left: middle of the ARMS and BOMB labels, and the empty row under them
        constexpr int32_t ARMS_X = 19;
        constexpr int32_t BOMB_X = 47;
        constexpr int32_t SLOT_Y = 8;

        constexpr int32_t SCORE_GAP = 4;

        // top left of the "1UP" label, the count starts after it and a space (8 px small font)
        constexpr ivec2   LIVES_POS{ 36, 24 };
        constexpr int32_t LIVES_COUNT_X = 32;

        // the text needs a layer of its own, inside one layer the draw order is the sprite pool's,
        // and that gets shuffled every time anything with a sprite is destroyed
        constexpr int32_t PANEL_LAYER = 100;
        constexpr int32_t TEXT_LAYER  = 101;
    }


    void Hud::create(Scene& scene)
    {
        create_hud_sprite(scene, anim::Hud::AmmoBombsPanel, PANEL_POS, PANEL_LAYER);

        const ivec2 arms = PANEL_POS + ivec2{ ARMS_X, SLOT_Y };
        const ivec2 bomb = PANEL_POS + ivec2{ BOMB_X, SLOT_Y };

        score_.create(scene, PANEL_POS - ivec2{ SCORE_GAP, 0 }, anim::Hud::MediumFont, 7, Align::Right, TEXT_LAYER);
        ammo_.create(scene, arms, anim::Hud::SmallFontYellow, 3, Align::Center, TEXT_LAYER);
        bombs_.create(scene, bomb, anim::Hud::SmallFontYellow, 2, Align::Center, TEXT_LAYER);

        // idek what that means, but it's in the game... so i made it ig
        create_hud_text(scene, anim::Hud::SmallFont, "1UP", LIVES_POS, TEXT_LAYER);
        lives_.create(scene,
            LIVES_POS + ivec2{ LIVES_COUNT_X, 0 },
            anim::Hud::SmallFontYellow, 2,
            Align::Left, TEXT_LAYER);

        inf_ = create_hud_sprite(scene, anim::Hud::SmallInfYellow, arms, TEXT_LAYER);
        scene.get<Transform>(inf_).position.x -= (int32_t)scene.get<Sprite>(inf_).size().x / 2;

        // the player never gets grenades, it's only there because the panel has a slot for it
        // TODO: add grenades if time
        bombs_.set(scene, 0);
    }

    void Hud::update(Scene& scene) const
    {
        const UserCtx&  u  = MetalSlug::user_ctx();
        const BodyData& bd = scene.get<BodyData>(u.player.body());

        score_.set(scene, u.player.score());
        lives_.set(scene, bd.lives);

        const bool pistol              = bd.weapon == WeaponType::Pistol;
        scene.get<Sprite>(inf_).hidden = !pistol;

        if (pistol) ammo_.hide(scene);
        else ammo_.set(scene, bd.ammo);
    }
}
