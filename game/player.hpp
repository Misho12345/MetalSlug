#pragma once
#include "mse/mse.hpp"


enum class LegsState : uint8_t { Stand, Walk, Air, Count };
enum class BodyState : uint8_t { Idle, Tired, Drinking, Count };

struct AttachedTo { mse::entity_id entity; };

struct Intent
{
    enum Bits : uint8_t
    {
        Jump = 1 << 0,
        Fire = 1 << 1,
        Drink = 1 << 2
    };

    uint8_t bits{};
    int8_t dir{ 0 };
};

class Player
{
public:
    explicit Player(mse::Scene& scene);

    static void register_machines();

    void update(mse::Scene& scene) const;
    void late_update(mse::Scene& scene) const;

private:
    mse::entity_id player_; // draws legs, physics
    mse::entity_id body_;   // draws body, follows legs
};