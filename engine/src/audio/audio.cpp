#include "mse/audio/audio.hpp"

#include <cassert>

namespace mse
{
    static constexpr ma_uint32 SFX_FLAGS =
            MA_SOUND_FLAG_DECODE |
            MA_SOUND_FLAG_NO_SPATIALIZATION |
            MA_SOUND_FLAG_NO_PITCH;

    // a few minutes of song decoded up front would be tens of MB
    static constexpr ma_uint32 MUSIC_FLAGS =
            MA_SOUND_FLAG_STREAM |
            MA_SOUND_FLAG_NO_SPATIALIZATION |
            MA_SOUND_FLAG_NO_PITCH;

    bool Audio::init()
    {
        initialised_ = ma_engine_init(nullptr, &engine_) == MA_SUCCESS;
        return initialised_;
    }

    Audio::~Audio()
    {
        if (!initialised_) return;

        stop_music();
        unload_all_sfx();
        ma_engine_uninit(&engine_);
        initialised_ = false;
    }

    sfx_id Audio::load_sfx(const char* path, const uint32_t voice_count)
    {
        assert(voice_count >= 1 && voice_count <= MAX_VOICES);
        if (!initialised_) return NO_SFX;

        unique_ptr<SfxPool> pool = make_unique<SfxPool>();
        if (ma_sound_init_from_file(
            &engine_, path, SFX_FLAGS,
            nullptr, nullptr, &pool->voices[0]) != MA_SUCCESS)
            return NO_SFX;

        pool->voice_count = 1;

        // copies share the first voice's decoded buffer, they only add a refcount
        for (uint32_t i = 1; i < voice_count; i++)
        {
            if (ma_sound_init_copy(&engine_, &pool->voices[0], SFX_FLAGS,
                nullptr, &pool->voices[i]) != MA_SUCCESS)
                break;

            pool->voice_count++;
        }

        pools_.push_back(std::move(pool));
        return (sfx_id)(pools_.size() - 1);
    }

    void Audio::play(const sfx_id id, const float volume)
    {
        if (!initialised_) return;

        if (id == NO_SFX) return;
        SfxPool& pool = *pools_[id];

        // first idle voice from the cursor on; if all are busy, restart the one at the cursor
        uint32_t pick = pool.next;
        for (uint32_t i = 0; i < pool.voice_count; i++)
        {
            uint32_t v = (pool.next + i) % pool.voice_count;
            if (!ma_sound_is_playing(&pool.voices[v]))
            {
                pick = v;
                break;
            }
        }

        pool.next = (pick + 1) % pool.voice_count;

        ma_sound& s = pool.voices[pick];
        ma_sound_set_volume(&s, volume);
        ma_sound_seek_to_pcm_frame(&s, 0);
        ma_sound_start(&s);
    }

    bool Audio::play_music(const char* path, const loop_music loop, const float volume)
    {
        stop_music();
        if (!initialised_) return false;

        if (ma_sound_init_from_file(&engine_, path, MUSIC_FLAGS, nullptr, nullptr, &music_) != MA_SUCCESS)
            return false;

        music_loaded_ = true;

        ma_sound_set_looping(&music_, loop ? MA_TRUE : MA_FALSE);
        ma_sound_set_volume(&music_, volume);
        ma_sound_start(&music_);
        return true;
    }

    void Audio::stop_music()
    {
        if (!music_loaded_) return;

        ma_sound_uninit(&music_);
        music_loaded_ = false;
    }

    void Audio::unload_all_sfx()
    {
        for (unique_ptr<SfxPool>& pool : pools_)
        {
            for (uint32_t i = 0; i < pool->voice_count; i++)
                ma_sound_uninit(&pool->voices[i]);
        }

        pools_.clear();
    }
}
