#pragma once
#include "mse/mse.hpp"

enum class UpgradeType : uint8_t
{
    RifleSpray, RifleFireball, // xp, ammo?
};


struct Upgrade final
{
    UpgradeType type;
    bool claimed{ false };
};
