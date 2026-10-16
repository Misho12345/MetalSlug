#pragma once
#include "mse/mse.hpp"

enum class GameState : uint8_t
{
    Intro,
    Gameplay,
    GameOver,
    Scoreboard,
    COUNT
};

using GameSM = mse::StateMachine<GameState>;

static const GameSM& game_sm()
{
    using B = GameSM::Behaviour;
    static constexpr GameSM sm
    {
        B{ nullptr, nullptr, nullptr }, // Intro
        B{ nullptr, nullptr, nullptr }, // Gameplay
        B{ nullptr, nullptr, nullptr }, // GameOver
        B{ nullptr, nullptr, nullptr }  // Scoreboard
    };

    return sm;
}


class MetalSlug final : public mse::App
{
public:
    void init() override;
    void update() override;
    void late_update() override;
};