#include "number.hpp"

namespace ms
{
    using namespace mse;

    entity_id create_hud_sprite(Scene& scene, const anim::Hud anim, const ivec2 top_left, const int32_t layer)
    {
        const entity_id e = scene.create_entity();

        Transform& tr = scene.set<Transform>(e);
        Sprite&    s  = scene.set<Sprite>(e, anim);

        s.parallax_factor = { 0.0f, 0.0f };
        s.layer           = layer;

        const ivec2 size = s.size();
        tr.position      = top_left + size / 2;

        return e;
    }

    void create_hud_text(Scene& scene, const anim::Hud font, const char* text, ivec2 top_left, const int32_t layer)
    {
        for (; *text; ++text)
        {
            const char ch = *text;

            const entity_id e = create_hud_sprite(scene, font, top_left, layer);
            Sprite&         s = scene.get<Sprite>(e);

            top_left.x += (int32_t)s.size().x;

            // still an entity so the advance comes from the font like every other character
            if (ch == ' ')
            {
                s.hidden = true;
                continue;
            }

            if (ch >= '0' && ch <= '9')
            {
                s.force_frame(FONT_FIRST_DIGIT + (uint32_t)(ch - '0'));
                continue;
            }

            assert(ch >= 'A' && ch <= 'Z' && "create_hud_text: the fonts only have A-Z and 0-9");
            s.force_frame((uint32_t)(ch - 'A'));
        }
    }


    void NumberDisplay::create(
        Scene&          scene,
        const ivec2     pos,
        const anim::Hud font,
        const uint8_t   digits,
        const Align     align,
        const int32_t   layer)
    {
        assert(digits > 0 && digits <= MAX_DIGITS);

        pos_   = pos;
        count_ = digits;
        align_ = align;

        max_ = 0;
        for (uint8_t i = 0; i < count_; ++i)
        {
            max_       = max_ * 10 + 9;
            digits_[i] = create_hud_sprite(scene, font, pos, layer);
        }

        hide(scene);
    }

    void NumberDisplay::set(Scene& scene, uint32_t value) const
    {
        value = glm::min(value, max_);

        uint8_t n = 1;
        for (uint32_t v = value / 10; v > 0; v /= 10) ++n;

        const ivec2 size = scene.get<Sprite>(digits_[0]).size();

        int32_t left = pos_.x;
        if (align_ == Align::Center) left -= n * size.x / 2;
        else if (align_ == Align::Right) left -= n * size.x;

        for (uint8_t i = 0; i < count_; ++i, value /= 10)
        {
            Sprite& s = scene.get<Sprite>(digits_[i]);

            s.hidden = i >= n;
            if (s.hidden) continue;

            s.force_frame(FONT_FIRST_DIGIT + value % 10);

            scene.get<Transform>(digits_[i]).position = {
                left + (n - 1 - i) * size.x + size.x / 2,
                pos_.y + size.y / 2
            };
        }
    }

    void NumberDisplay::hide(Scene& scene) const
    {
        for (uint8_t i = 0; i < count_; ++i)
            scene.get<Sprite>(digits_[i]).hidden = true;
    }
}
