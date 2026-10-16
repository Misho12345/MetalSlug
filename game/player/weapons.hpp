#pragma once
#include "mse/mse.hpp"

namespace player
{
    using namespace mse;
    using namespace mse::literals;

    enum class WeaponType : uint8_t
    {
        Pistol,
        RifleSpray,
        RifleFireball,
        Count
    };

    struct WeaponData final
    {
        uint16_t refire_ticks; // fire cooldown
        uint8_t  burst;        // bullets per press
        uint8_t  burst_ticks;  // coolldown between the bullets of a burst
        float    speed;
    };

    // indexed by WeaponType
    constexpr WeaponData WEAPONS[]
    {
        {  6, 1, 0, 250.0f }, // Pistol
        {  4, 4, 4, 300.0f }, // RifleSpray, a press mid burst starts a new one
        { 18, 1, 0, 180.0f }, // RifleFireball, 18 = its whole shoot animation
    };

    static_assert(std::size(WEAPONS) == (size_t)WeaponType::Count);

    // per bullet of a burst, so the spray isn't a straight line
    inline const float SPRAY_SPREAD[]{ 0.0_deg, 4.0_deg, -3.0_deg, 2.0_deg };
}
