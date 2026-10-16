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



class MetalSlug final : public mse::App
{
public:
    void init() override;
    void update() override;
    void late_update() override;

private:
    using SM = mse::StateMachine<GameState>;
    using B = SM::Behaviour;

    static constexpr uint32_t TRANSITION_DUR = 512;

    static void intro_enter(const SM::Ctx& ctx);
    static GameState on_intro(const SM::Ctx& ctx);
    static void intro_exit(const SM::Ctx& ctx);

    static void gameplay_enter(const SM::Ctx& ctx);
    static void gameplay_exit(const SM::Ctx& ctx);

    static void game_over_enter(const SM::Ctx& ctx);
    static GameState on_game_over(const SM::Ctx& ctx);

    static void scoreboard_enter(const SM::Ctx& ctx);
    static GameState on_scoreboard(const SM::Ctx& ctx);

    static constexpr SM sm
    {
        B{ intro_enter, on_intro, intro_exit }, // Intro
        B{ gameplay_enter, nullptr, gameplay_exit }, // Gameplay
        B{ game_over_enter, on_game_over, nullptr }, // GameOver
        B{ scoreboard_enter, on_scoreboard, nullptr }  // Scoreboard
    };

    mse::entity_id app_;
};