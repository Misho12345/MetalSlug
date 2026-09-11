#pragma once
#include "api.hpp"
#include "pch.hpp"

namespace mse
{
    struct SpriteAnimationData final
    {
        glm::vec2 offset;
        glm::vec2 image_size;
        uint32_t  frame_count;
        uint32_t  atlas_idx;
    };

    struct SpriteDesc final
    {
        glm::vec2 pos{ 0.0f };
        glm::vec2 size{ 1.0f };
        float     rot{ 0.0f };
    };

    class MSE_API Sprite final
    {
    public:
        template <anim::sprite_enum E>
        static Sprite& create(const SpriteDesc& desc = {})
        {
            return create(anim::sprite_id<E>, desc);
        }

        void destroy() const;

        template <anim::sprite_enum E>
        static void set_frame_count(E anim_id, const uint32_t count)
        {
            set_frame_count(anim::info(anim_id), count);
        }

        template <anim::sprite_enum E>
        void play(E anim_id) { play(static_cast<int>(anim_id)); }

        void update(float dt);

        glm::vec2 pos() const { return pos_; }
        glm::vec2 size() const { return size_; }
        float     rot() const { return rot_; }

        void set_pos(const glm::vec2 pos)
        {
            pos_  = pos;
            dirty_ = true;
        }

        void set_size(const glm::vec2 size)
        {
            size_ = size;
            dirty_ = true;
        }

        void set_rot(const float rot)
        {
            rot_  = rot;
            dirty_ = true;
        }

        float time() const { return time_; }
        uint32_t frame() const { return frame_; }
        anim::Info info() const { return info_; }

        bool dirty() const { return dirty_; }

        float frame_dur{ 0.1f };

    private:
        Sprite(int sprite_id, const SpriteDesc& desc);

        static Sprite& create(int sprite_id, const SpriteDesc& desc);
        static void set_frame_count(anim::Info info, uint32_t count);

        void play(int anim_id = -1);
        void pause();

        glm::vec2 pos_;
        glm::vec2 size_;
        float     rot_;

        float    time_{};
        uint32_t frame_{};

        anim::Info info_{};

        bool paused_{ false };
        bool dirty_{ false };

        friend class SpriteManager;
    };
}
