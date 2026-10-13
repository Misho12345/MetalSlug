#pragma once
#include "mse/mse.hpp"


enum class BodyState : uint8_t
{
    Ready, Shooting,
    Transition, // turning, crouching down, standing up, raising / lowering the gun: no shooting meanwhile
    // Melee, Throwing, Dead,
    Count
};

enum class LegsState : uint8_t
{
    Stand, Walk, Air, Crouch,
    Count
};


enum class WeaponType : uint8_t
{
    Pistol,
    // later more
    Count
};


class Player
{
public:
    explicit Player(mse::Scene& scene);

    static void register_machines();

    void update(mse::Scene& scene) const;
    void late_update(mse::Scene& scene) const;

    struct Intent
    {
        enum Bits : uint8_t
        {
            Jump = 1 << 0,
            Fire = 1 << 1,
            // Throw = 1 << 2,
            // Drink = 1 << 3,
        };

        uint8_t down{};
        uint8_t just_pressed{};

        glm::i8vec2 dir{ 0 };
    };

private:
    mse::entity_id legs_;
    mse::entity_id body_;
};