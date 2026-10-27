#include "debrief.hpp"
#include "number.hpp"
#include "metal_slug.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        // ~0.7 s of fading, then ~4 s to read the panel
        constexpr uint16_t DIM_TICKS = 40;
        constexpr uint16_t END_TICKS = DIM_TICKS + 240;
        constexpr uint8_t  DIM_ALPHA = 140;

        // the "X1000" on the panel is just text, change both
        constexpr uint32_t POW_POINTS = 1000;

        // frames 9-12 of the fade out strip are the salute and the wave, played back and forth
        constexpr uint32_t WAVE_FIRST = 9;
        constexpr uint32_t WAVE_COUNT = 4;
        constexpr uint16_t WAVE_TICKS = 8;

        // below the hud, which stays bright like in the original
        constexpr int32_t DIM_LAYER   = 99;
        constexpr int32_t FRAME_LAYER = 102;
        constexpr int32_t TEXT_LAYER  = 103;

        // top lefts in screen pixels, eyeballed from a screenshot of the original
        constexpr ivec2 TITLE_POS{ 56, 36 };
        constexpr ivec2 FRAME_POS{ 6, 54 };

        // from the frame's top left. The numbers are right aligned, so those x's are their right edges
        constexpr ivec2 POW_POS{ 5, 4 };
        constexpr ivec2 P1_POS{ 48, 6 };
        constexpr ivec2 COUNT_POS{ 72, 28 };
        constexpr ivec2 TIMES_POS{ 74, 36 }; // small font, so 8 lower to sit on the same line as the count
        constexpr ivec2 BONUS_POS{ 112, 50 };
        constexpr ivec2 NONE_POS{ 16, 70 };

        // under the line
        constexpr ivec2 SCORE_LABEL_POS{ 16, 102 };
        constexpr ivec2 SCORE_POS{ 112, 124 };
    }


    void Debrief::create(Scene& scene)
    {
        dim_ = create_hud_sprite(scene, anim::Hud::Dim, {}, DIM_LAYER);
        scene.get<Sprite>(dim_).color.a = 0;
    }

    void Debrief::update(Scene& scene, const uint16_t ticks)
    {
        if (ticks <= DIM_TICKS)
            scene.get<Sprite>(dim_).color.a = (uint8_t)(DIM_ALPHA * ticks / DIM_TICKS);

        if (ticks == DIM_TICKS) show(scene);
        if (ticks < DIM_TICKS) return;

        // 0 1 2 3 2 1 0 1 ...
        constexpr uint32_t period = 2 * (WAVE_COUNT - 1);

        uint32_t step = (uint32_t)(ticks - DIM_TICKS) / WAVE_TICKS % period;
        if (step >= WAVE_COUNT) step = period - step;

        scene.get<Sprite>(pow_).force_frame(WAVE_FIRST + step);
    }

    bool Debrief::done(const uint16_t ticks) { return ticks >= END_TICKS; }

    void Debrief::show(Scene& scene)
    {
        Player& player = MetalSlug::user_ctx().player;

        const uint32_t freed = player.pows_freed();
        const uint32_t bonus = freed * POW_POINTS;

        player.add_score(bonus);

        create_hud_text(scene, anim::Hud::MediumFont, "RECAPTURED PRISONER", TITLE_POS, TEXT_LAYER);
        create_hud_sprite(scene, anim::Hud::FramePanel, FRAME_POS, FRAME_LAYER);

        // create_hud_sprite only takes hud sprites, so the same thing by hand
        pow_ = scene.create_entity();
        {
            Transform& tr = scene.set<Transform>(pow_);
            Sprite&    s  = scene.set<Sprite>(pow_, anim::Pow::FadeOut);

            s.parallax_factor = { 0.0f, 0.0f };
            s.layer           = TEXT_LAYER;

            const ivec2 size = s.size();
            tr.position      = FRAME_POS + POW_POS + size / 2;
        }

        create_hud_text(scene, anim::Hud::BigFont, "P1", FRAME_POS + P1_POS, TEXT_LAYER);
        create_hud_text(scene, anim::Hud::SmallFont, "X1000", FRAME_POS + TIMES_POS, TEXT_LAYER);

        // set once and never touched again, so the displays don't need keeping,
        // their digits are entities like everything else and go when the scene is cleared
        NumberDisplay count;
        count.create(scene, FRAME_POS + COUNT_POS, anim::Hud::BigFont, 3, Align::Right, TEXT_LAYER);
        count.set(scene, freed);

        NumberDisplay points;
        points.create(scene, FRAME_POS + BONUS_POS, anim::Hud::BigFont, 6, Align::Right, TEXT_LAYER);
        points.set(scene, bonus);

        if (freed == 0)
            create_hud_text(scene, anim::Hud::MediumFont, "NO PRISONER", FRAME_POS + NONE_POS, TEXT_LAYER);

        create_hud_text(scene, anim::Hud::MediumFont, "TOTAL SCORE", FRAME_POS + SCORE_LABEL_POS, TEXT_LAYER);

        NumberDisplay score;
        score.create(scene, FRAME_POS + SCORE_POS, anim::Hud::MediumFont, 7, Align::Right, TEXT_LAYER);
        score.set(scene, player.score());
    }
}
