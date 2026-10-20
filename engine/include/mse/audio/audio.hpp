#pragma once

#include "mse/api.hpp"

#include <miniaudio.h>

namespace mse
{
    using sfx_id = uint32_t;
    static constexpr sfx_id NO_SFX = 0xFFFFFFFF;

    struct GameCtx;

    class MSE_API Audio final
    {
        static constexpr uint32_t MAX_VOICES = 8;

    public:
        ~Audio();

        bool init();

        sfx_id load_sfx(const char* path, uint32_t voice_count = 4);
        void play(sfx_id id, float volume = 1.0f);

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

        vector<unique_ptr<SfxPool>> pools_{ nullptr };
        vector<DecodedClip> decoded_{ nullptr };

        friend ::mse::GameCtx;
    };
}