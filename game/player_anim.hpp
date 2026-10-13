#pragma once
#include "mse/mse.hpp"

// all player animation data
namespace player_anim
{
    using namespace mse;
    using anim::player::Body;
    using anim::player::Legs;

    // for Legs::SideJump and Body::PistolSideJump
    constexpr uint8_t JUMP_FRAMES[]{ 4, 8, 12, 12, 8, 4 };

    static_assert(std::size(JUMP_FRAMES) == anim::frame_count(anim::info(Legs::SideJump)));
    static_assert(std::size(JUMP_FRAMES) == anim::frame_count(anim::info(Body::PistolSideJump)));

    enum class End : uint8_t { Hold, Loop, Next };

    // origin: top left corner of the sprite when facing right, from the top left corner of the legs' 21x16 box
    // (the idle legs sprite is exactly that box). Idle comes from lining the two halves up with the full body
    // sprites, the rest is by eye

    struct LegsAnim final
    {
        uint8_t ticks; // per frame, 0 = doesn't advance
        ivec2   origin;
        int32_t waist_lift{ 0 };
        bool    loop{ false };
    };

    constexpr LegsAnim data(const Legs a)
    {
        switch (a)
        {
            case Legs::Idle:     return { 0, { 0, 0 }, 0, true };
            case Legs::Walk:     return { 4, { -4, -4 }, 3, true };
            case Legs::InAir:    return { 4, { 0, -4 }, 2 };
            case Legs::SideJump: return { 0, { -10, -4 }, 3 }; // JUMP_FRAMES
            default: assert(!"not implemented"); return {};
        }
    }

    struct BodyAnim final
    {
        uint8_t ticks;
        ivec2   origin;
        End     end{ End::Hold };
        bool    full_body{ false }; // the sprite includes the legs
        Body    next{};             // for End::Next
        uint8_t reversed_ticks{ 0 }; // per frame when played backwards, 0 = it never is
    };

    constexpr BodyAnim data(const Body a)
    {
        switch (a)
        {
            case Body::PistolIdle:     return { 8, {  0, -23 }, End::Loop };
            case Body::PistolWalk:     return { 4, { -2, -23 }, End::Loop };
            case Body::PistolWalkStop: return { 7, { -1, -21 }, End::Hold, true };
            case Body::PistolTurn:     return { 4, { -1, -22 }, End::Hold, true }; // 1 frame, the turn ends when it's done

            case Body::PistolInAir:    return { 4, { -1, -25 } };
            case Body::PistolSideJump: return { 0, { -1, -23 } }; // JUMP_FRAMES

            case Body::PistolLookUp:     return { 4, { -1, -24 }, End::Hold, false, {}, 2 };
            case Body::PistolLookUpIdle: return { 8, {  0, -24 }, End::Loop };
            case Body::PistolLookUpTurn: return { 4, {  0, -24 }, End::Hold, true };
            case Body::PistolLookDown:   return { 4, {  1, -23 } }; // only in air

            case Body::PistolShoot:     return { 2, {  0, -24 } };
            case Body::PistolJumpShoot: return { 2, {  0, -24 } };
            case Body::PistolShootUp:   return { 2, { -1, -61 } };
            case Body::PistolShootDown: return { 2, {  2, -23 } }; // only in air

            case Body::CrouchPistol:           return { 4, {  0, -25 }, End::Hold, true, {}, 2 };
            case Body::CrouchPistolIdle:       return { 8, { -4,  -8 }, End::Loop, true };
            case Body::CrouchPistolWalk:       return { 4, { -4,  -8 }, End::Loop, true };
            case Body::CrouchPistolTurn:       return { 4, {  0, -10 }, End::Hold, true };
            case Body::CrouchPistolShoot:      return { 2, { -1, -13 }, End::Next, true, Body::CrouchPistolAfterShoot };
            case Body::CrouchPistolAfterShoot: return { 4, {  1, -11 }, End::Hold, true };

            // TODO: knife, throw, drink, reload
            default: assert(!"not implemented"); return {};
        }
    }

    inline void play(Sprite& sprite, const Legs a)
    {
        const LegsAnim    d    = data(a);
        if (a == Legs::SideJump) sprite.play(a, JUMP_FRAMES, is_reversed::no, d.loop);
        else sprite.play(a, d.ticks, is_reversed::no, d.loop);
    }

    // always from the start, late_update only calls it when the animation changes
    inline void play(Sprite& sprite, const Body a, const bool reversed = false)
    {
        const BodyAnim d    = data(a);
        const bool     loop = d.end == End::Loop;

        if (reversed)
        {
            assert(d.reversed_ticks != 0);
            sprite.play(a, d.reversed_ticks, is_reversed::yes, should_loop::no, should_restart_if_same::yes);
        }
        else if (a == Body::PistolSideJump) sprite.play(a, JUMP_FRAMES, is_reversed::no, loop, should_restart_if_same::yes);
        else sprite.play(a, d.ticks, is_reversed::no, loop, should_restart_if_same::yes);
    }


    // what the body looks like, without the weapon
    enum class Pose : uint8_t
    {
        Idle, Walk, WalkStop, Turn,
        InAir, SideJump,
        LookUp, LookUpIdle, LookUpTurn, LookDown,
        Shoot, JumpShoot, ShootUp, ShootDown,
        CrouchDown, CrouchIdle, CrouchWalk, CrouchTurn, CrouchShoot,
        LookUpEnd, CrouchUp,
        Count
    };

    struct PoseAnim final
    {
        Body anim;
        bool reversed{ false };

        constexpr bool operator==(const PoseAnim&) const = default;
    };

    // in the order of Pose, a new weapon is one more of these
    constexpr PoseAnim PISTOL_POSES[]
    {
        { Body::PistolIdle }, { Body::PistolWalk }, { Body::PistolWalkStop }, { Body::PistolTurn },
        { Body::PistolInAir }, { Body::PistolSideJump },
        { Body::PistolLookUp }, { Body::PistolLookUpIdle }, { Body::PistolLookUpTurn }, { Body::PistolLookDown },
        { Body::PistolShoot }, { Body::PistolJumpShoot }, { Body::PistolShootUp }, { Body::PistolShootDown },
        { Body::CrouchPistol }, { Body::CrouchPistolIdle }, { Body::CrouchPistolWalk }, { Body::CrouchPistolTurn }, { Body::CrouchPistolShoot },
        { Body::PistolLookUp, true }, { Body::CrouchPistol, true },
    };

    static_assert(std::size(PISTOL_POSES) == (size_t)Pose::Count);

    // indexed by WeaponType
    constexpr const PoseAnim* POSE_ANIM[]{ PISTOL_POSES };
}