#pragma once
#include "user_ctx.hpp"
#include "mse/mse.hpp"

namespace ms
{
    enum class GameState : uint8_t
    {
        Intro,
        Gameplay,
        Debrief,
        GameOver,
        COUNT
    };

    class MetalSlug final : public mse::App
    {
    public:
        void init() override;
        void update() override;
        void late_update() override;

        static UserCtx& user_ctx();

    private:
        using SM = mse::StateMachine<GameState>;
        using B = SM::Behaviour;

        static constexpr uint32_t TRANSITION_DUR = 512;

        static void intro_enter(const SM::Ctx& ctx);
        static GameState on_intro(const SM::Ctx& ctx);
        static void intro_exit(const SM::Ctx& ctx);

        static void gameplay_enter(const SM::Ctx& ctx);

        static void debrief_enter(const SM::Ctx& ctx);
        static GameState on_debrief(const SM::Ctx& ctx);

        static void game_over_enter(const SM::Ctx& ctx);
        static GameState on_game_over(const SM::Ctx& ctx);

        static constexpr SM sm
        {
            B{ intro_enter, on_intro, intro_exit }, // Intro
            B{ gameplay_enter, nullptr, nullptr }, // Gameplay
            B{ debrief_enter, on_debrief, nullptr }, // Debrief
            B{ game_over_enter, on_game_over, nullptr } // GameOver
        };

        UserCtx user_ctx_;
        mse::entity_id app_;
    };
}

using ms::MetalSlug;