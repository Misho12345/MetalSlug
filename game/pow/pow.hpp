#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class PowState : uint8_t
    {
        Idle,
        Free,
        FadeOut,
        COUNT
    };


    class POWManager final
    {
    public:
        enum class Kind : uint8_t { Sitting, Tree };

        void register_machine() const;

        mse::entity_id spawn(mse::Scene& scene, mse::ivec2 pos, Kind kind) const;

    private:
        using PowSM = mse::StateMachine<PowState>;

        static constexpr uint8_t IDLE_TICKS = 5;
        static constexpr uint8_t FREE_TICKS = 5;
        static constexpr uint8_t FADE_TICKS = 6;
        static constexpr uint8_t TREE_TICKS = 6;

        static constexpr uint32_t FADE_TOTAL_TICKS = FADE_TICKS * mse::anim::frame_count(mse::anim::info(mse::anim::Pow::FadeOut));

        // middle of each strip in px, from the idle one's.
        // The animations are weird and not centered and that's the best I could do to align them to be good enough
        static constexpr int32_t SITTING_FREE_X = 0;
        static constexpr int32_t SITTING_FADE_X = 1;
        static constexpr int32_t TREE_FREE_X    = 2;
        static constexpr int32_t TREE_FADE_X    = 0;
        static constexpr int32_t TREE_DESTROY_X = 24;

        // left, the tree falls apart to the right (TREE_DESTROY_X)
        static constexpr int32_t DROP_X = -20;


        struct Data final
        {
            Kind kind;
            mse::ivec2 base;
            bool shot{ false };
        };


        struct StateManager;

        static const PowSM& sm();
    };
}