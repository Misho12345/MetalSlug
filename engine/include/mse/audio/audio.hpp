#pragma once

#include "mse/api.hpp"
#include "mse/pch.hpp"

#include <miniaudio.h>

namespace mse
{
    using sfx_id = uint32_t;
    static constexpr sfx_id NO_SFX = 0xFFFFFFFF;

    using loop_music = bool_value<struct loop_music_tag>;

    struct GameCtx;

    class MSE_API Audio final
    {
        static constexpr uint32_t MAX_VOICES = 8;

    public:
        ~Audio();

        bool init();

        sfx_id load_sfx(const char* path, uint32_t voice_count = 4);
        void play(sfx_id id, float volume = 1.0f);

        bool play_music(const char* path, loop_music loop = loop_music::yes, float volume = 1.0f);
        void stop_music();

        ma_engine& engine() { return engine_; }

    private:
        Audio() = default;

        struct SfxPool
        {
            ma_sound voices[MAX_VOICES];
            uint32_t voice_count = 0;
            uint32_t next = 0;
        };

        struct DecodedClip
        {
            string name;
            void* frames = nullptr;
        };

        void unload_all_sfx();

        ma_engine engine_{};
        bool initialised_ = false;

        ma_sound music_{};
        bool music_loaded_ = false;

        vector<unique_ptr<SfxPool>> pools_{ nullptr };
        vector<DecodedClip> decoded_{ nullptr };

        friend ::mse::GameCtx;
    };
}