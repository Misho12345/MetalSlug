#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class Align : uint8_t { Left, Center, Right };

    // the font strips go A-Z, then 0-9
    inline constexpr uint32_t FONT_FIRST_DIGIT = 26;

    // stays on the screen instead of moving with the level, top_left is in screen pixels
    mse::entity_id create_hud_sprite(mse::Scene& scene, mse::anim::Hud anim, mse::ivec2 top_left, int32_t layer);

    // for labels that never change, only A-Z, 0-9 and spaces
    void create_hud_text(mse::Scene& scene, mse::anim::Hud font, const char* text, mse::ivec2 top_left, int32_t layer);

    // a number made out of one sprite per digit, each one forced onto its frame of a font strip.
    // A little inefficient, a whole entity for every digit and all of them redone on every set,
    // but it's the simplest thing that works with what the engine already has, so whatever
    class NumberDisplay final
    {
    public:
        // anything longer shows as all nines, and 9 of them still fit in a uint32_t
        static constexpr uint8_t MAX_DIGITS = 9;

        // pos.x is the left edge, the middle or the right edge depending on align, pos.y is the top
        void create(mse::Scene& scene, mse::ivec2 pos, mse::anim::Hud font, uint8_t digits, Align align, int32_t layer);

        void set(mse::Scene& scene, uint32_t value) const;
        void hide(mse::Scene& scene) const;

    private:
        mse::entity_id digits_[MAX_DIGITS]; // least significant first
        mse::ivec2     pos_{};

        uint32_t max_{};
        uint8_t  count_{};
        Align    align_{};
    };
}
