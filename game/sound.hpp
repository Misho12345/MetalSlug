#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class Sfx : uint8_t
    {
        Title,        // "Metal Slug!"
        MissionStart, // "Mission 1, start!"
        GameOver,     // "Game over"

        Pistol,
        MachineGun, // a whole burst, played once per burst and not per bullet
        FireShot,

        // the announcer on picking up a weapon, plus the click
        HeavyMachineGun,
        FlameShot,
        Pickup,

        PowFreed,
        Knife,

        COUNT
    };

    enum class Music : uint8_t
    {
        Gameplay,
        Debrief,
        GameOver,
        COUNT
    };

    class Sounds final
    {
    public:
        void load();

        void play(Sfx sfx) const;

        // a random one of the variations
        void explosion() const;
        void rebel_death() const;

        void music(Music music) const;
        void stop_music() const;

    private:
        static constexpr uint32_t EXPLOSIONS   = 3;
        static constexpr uint32_t REBEL_DEATHS = 9;

        mse::sfx_id sfx_[(size_t)Sfx::COUNT]{};
        mse::sfx_id explosions_[EXPLOSIONS]{};
        mse::sfx_id deaths_[REBEL_DEATHS]{};
    };
}
