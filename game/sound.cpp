#include "sound.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        struct Clip
        {
            const char* path;
            uint32_t    voices; // how many can overlap
            float       volume;
        };

        // indexed by Sfx. The volumes even out the files, the announcer ones are much quieter than the guns
        constexpr Clip SFX[]
        {
            { "assets/audio/sfx/title.wav", 1, 1.6f },           // Title, a wav copy of title.ogg, the engine's miniaudio has no vorbis
            { "assets/audio/sfx/mission1.mp3", 1, 1.0f },        // MissionStart
            { "assets/audio/sfx/sgameover.mp3", 1, 1.0f },       // GameOver
            { "assets/audio/sfx/pistol.wav", 4, 0.6f },          // Pistol
            { "assets/audio/sfx/MGShoot.wav", 3, 0.45f },        // MachineGun
            { "assets/audio/sfx/firehit.mp3", 2, 0.5f },         // FireShot
            { "assets/audio/sfx/heavymachinegun.mp3", 1, 1.0f }, // HeavyMachineGun
            { "assets/audio/sfx/flameshot.mp3", 1, 1.0f },       // FlameShot
            { "assets/audio/sfx/Magazine.wav", 2, 0.6f },        // Pickup
            { "assets/audio/sfx/Prisioner.mp3", 2, 1.2f },       // PowFreed
            { "assets/audio/sfx/knife1.wav", 3, 0.6f },          // Knife
        };

        static_assert(std::size(SFX) == (size_t)Sfx::COUNT);

        constexpr float EXPLOSION_VOLUME = 0.6f;
        constexpr float DEATH_VOLUME     = 0.7f;

        struct Track
        {
            const char* path;
            loop_music  loop;
        };

        // indexed by Music. Low enough to hear the guns over it
        constexpr Track MUSIC[]
        {
            { "assets/audio/bgm/MainTheme.mp3", loop_music::yes },       // Gameplay
            { "assets/audio/bgm/CarryOut.mp3", loop_music::no },         // Debrief
            { "assets/audio/bgm/1-12. Gravestone.mp3", loop_music::no }, // GameOver
        };

        static_assert(std::size(MUSIC) == (size_t)Music::COUNT);

        constexpr float MUSIC_VOLUME = 0.4f;

        Audio& audio() { return App::ctx().audio; }

        sfx_id load_clip(const char* path, const uint32_t voices)
        {
            const sfx_id id = audio().load_sfx(path, voices);
            if (id == NO_SFX) printf("failed to load %s\n", path);
            return id;
        }
    }


    void Sounds::load()
    {
        for (size_t i = 0; i < (size_t)Sfx::COUNT; ++i)
            sfx_[i] = load_clip(SFX[i].path, SFX[i].voices);

        char path[64];

        for (uint32_t i = 0; i < EXPLOSIONS; ++i)
        {
            snprintf(path, sizeof(path), "assets/audio/sfx/explosion%u.wav", i + 1);
            explosions_[i] = load_clip(path, 2);
        }

        for (uint32_t i = 0; i < REBEL_DEATHS; ++i)
        {
            snprintf(path, sizeof(path), "assets/audio/sfx/rebel_dying/dying%u.wav", i + 1);
            deaths_[i] = load_clip(path, 2);
        }
    }

    void Sounds::play(const Sfx sfx) const { audio().play(sfx_[(size_t)sfx], SFX[(size_t)sfx].volume); }

    void Sounds::explosion() const
    {
        audio().play(explosions_[App::ctx().rng.range(0u, EXPLOSIONS - 1)], EXPLOSION_VOLUME);
    }

    void Sounds::rebel_death() const
    {
        audio().play(deaths_[App::ctx().rng.range(0u, REBEL_DEATHS - 1)], DEATH_VOLUME);
    }

    void Sounds::music(const Music music) const
    {
        const Track& t = MUSIC[(size_t)music];
        if (!audio().play_music(t.path, t.loop, MUSIC_VOLUME)) printf("failed to play %s\n", t.path);
    }

    void Sounds::stop_music() const { audio().stop_music(); }
}
