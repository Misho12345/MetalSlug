#pragma once
#include "mse/mse.hpp"

// all player animation data
namespace ms
{
    using namespace mse;
    using anim::player::Body;
    using anim::player::Legs;

    // for Legs::SideJump and Body::PistolSideJump
    constexpr uint8_t JUMP_FRAMES[]{ 4, 8, 12, 12, 8, 4 };

    static_assert(std::size(JUMP_FRAMES) == anim::frame_count(anim::info(Legs::SideJump)));
    static_assert(std::size(JUMP_FRAMES) == anim::frame_count(anim::info(Body::PistolSideJump)));
    static_assert(std::size(JUMP_FRAMES) == anim::frame_count(anim::info(Body::RifleSideJump)));

    enum class End : uint8_t { Hold, Loop, Next };


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

            case Body::PistolCrouch:           return { 4, {  0, -25 }, End::Hold, true, {}, 2 };
            case Body::PistolCrouchIdle:       return { 8, { -4,  -8 }, End::Loop, true };
            case Body::PistolCrouchWalk:       return { 4, { -4,  -8 }, End::Loop, true };
            case Body::PistolCrouchTurn:       return { 4, {  0, -10 }, End::Hold, true };
            case Body::PistolCrouchShoot:      return { 2, { -1, -13 }, End::Next, true, Body::PistolCrouchAfterShoot };
            case Body::PistolCrouchAfterShoot: return { 8, {  1, -11 }, End::Loop, true };

            // rifle: pixel matched against the pistol ones (same head and arms), the shots against the pose they start from

            case Body::RifleIdle:     return { 8, {  0, -23 }, End::Loop };
            case Body::RifleWalk:     return { 4, { -2, -24 }, End::Loop };
            case Body::RifleWalkStop: return { 7, { -1, -21 }, End::Hold, true };
            case Body::RifleTurn:     return { 4, { -1, -22 }, End::Hold, true };

            case Body::RifleInAir:    return { 4, {  1, -23 } };
            case Body::RifleSideJump: return { 0, { -1, -23 } }; // JUMP_FRAMES

            case Body::RifleLookUp:     return { 4, { -1, -28 }, End::Hold, false, {}, 2 };
            case Body::RifleLookUpIdle: return { 8, {  0, -31 }, End::Loop };
            case Body::RifleLookUpTurn: return { 4, {  0, -31 }, End::Hold, true };
            case Body::RifleLookDown:   return { 4, { -2, -20 } }; // only in air

            // 4 frames at 1 tick, one loop per bullet of the burst
            case Body::RifleSprayShoot:     return { 2, { -1, -23 } };
            case Body::RifleSprayShootUp:   return { 2, { -1, -67 } };
            case Body::RifleSprayShootDown: return { 2, { -2, -19 } }; // only in air

            case Body::RifleSprayShootDiagUp:   return { 2, { -1, -60 } }; // head matched against RifleSprayShoot
            case Body::RifleSprayShootDiagDown: return { 2, { -2, -20 } }; // first frame is RifleLookDown's

            case Body::RifleFireballShoot:     return { 3, {  0, -23 } };
            case Body::RifleFireballShootUp:   return { 3, { -1, -44 } };
            case Body::RifleFireballShootDown: return { 3, { -2, -19 } }; // only in air

            case Body::RifleCrouch:              return { 4, {  0, -25 }, End::Hold, true, {}, 2 };
            case Body::RifleCrouchIdle:          return { 8, { -4,  -8 }, End::Loop, true };
            case Body::RifleCrouchWalk:          return { 4, { -4,  -8 }, End::Loop, true };
            case Body::RifleCrouchTurn:          return { 4, { -1, -10 }, End::Hold, true };
            case Body::RifleCrouchSprayShoot:    return { 2, { -3, -12 }, End::Next, true, Body::RifleCrouchAfterShoot };
            case Body::RifleCrouchFireballShoot: return { 3, { -1, -17 }, End::Next, true, Body::RifleCrouchAfterShoot };
            case Body::RifleCrouchAfterShoot:    return { 8, {  1, -11 }, End::Loop, true };

            case Body::DeathByStabbing: return { 4, { -3, -24 }, End::Hold, true };
            case Body::DeathByGrenade:  return { 4, { -1, -20 }, End::Hold, true };

            // TODO: knife, throw, drink, reload if time
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
            sprite.play(a, d.reversed_ticks, is_reversed::yes, should_loop::no, restart_if_same::yes);
        }
        else if (a == Body::PistolSideJump || a == Body::RifleSideJump)
        {
            sprite.play(a, JUMP_FRAMES, is_reversed::no, loop, restart_if_same::yes);
        }
        else sprite.play(a, d.ticks, is_reversed::no, loop, restart_if_same::yes);
    }


    // what the body looks like, without the weapon
    enum class Pose : uint8_t
    {
        Idle, Walk, WalkStop, Turn,
        InAir, SideJump,
        LookUp, LookUpIdle, LookUpTurn, LookDown,
        Shoot, JumpShoot, ShootUp, ShootDown,
        ShootDiagUp, ShootDiagDown, // spray only, the other two just reuse up/down
        CrouchDown, CrouchIdle, CrouchWalk, CrouchTurn, CrouchShoot, CrouchAfterShoot,
        LookUpEnd, CrouchUp,
        DeathByStabbing, DeathByGrenade,
        COUNT
    };

    struct PoseAnim final
    {
        Body anim;
        bool reversed{ false };
        
        constexpr bool operator==(const PoseAnim&) const = default;
    };

    // in the order of Pose
    constexpr PoseAnim PISTOL_POSES[]
    {
        { Body::PistolIdle }, { Body::PistolWalk }, { Body::PistolWalkStop }, { Body::PistolTurn },
        { Body::PistolInAir }, { Body::PistolSideJump },
        { Body::PistolLookUp }, { Body::PistolLookUpIdle }, { Body::PistolLookUpTurn }, { Body::PistolLookDown },
        { Body::PistolShoot }, { Body::PistolJumpShoot }, { Body::PistolShootUp }, { Body::PistolShootDown },
        { Body::PistolShootUp }, { Body::PistolShootDown },
        { Body::PistolCrouch }, { Body::PistolCrouchIdle }, { Body::PistolCrouchWalk }, { Body::PistolCrouchTurn }, { Body::PistolCrouchShoot }, { Body::PistolCrouchAfterShoot },
        { Body::PistolLookUp, true }, { Body::PistolCrouch, true },
        { Body::DeathByStabbing }, { Body::DeathByGrenade },
    };

    // the rifle has no jump shoot, the standing one goes over the air legs
    constexpr PoseAnim SPRAY_POSES[]
    {
        { Body::RifleIdle }, { Body::RifleWalk }, { Body::RifleWalkStop }, { Body::RifleTurn },
        { Body::RifleInAir }, { Body::RifleSideJump },
        { Body::RifleLookUp }, { Body::RifleLookUpIdle }, { Body::RifleLookUpTurn }, { Body::RifleLookDown },
        { Body::RifleSprayShoot }, { Body::RifleSprayShoot }, { Body::RifleSprayShootUp }, { Body::RifleSprayShootDown },
        { Body::RifleSprayShootDiagUp }, { Body::RifleSprayShootDiagDown },
        { Body::RifleCrouch }, { Body::RifleCrouchIdle }, { Body::RifleCrouchWalk }, { Body::RifleCrouchTurn }, { Body::RifleCrouchSprayShoot }, { Body::RifleCrouchAfterShoot },
        { Body::RifleLookUp, true }, { Body::RifleCrouch, true },
        { Body::DeathByStabbing }, { Body::DeathByGrenade },
    };

    constexpr PoseAnim FIREBALL_POSES[]
    {
        { Body::RifleIdle }, { Body::RifleWalk }, { Body::RifleWalkStop }, { Body::RifleTurn },
        { Body::RifleInAir }, { Body::RifleSideJump },
        { Body::RifleLookUp }, { Body::RifleLookUpIdle }, { Body::RifleLookUpTurn }, { Body::RifleLookDown },
        { Body::RifleFireballShoot }, { Body::RifleFireballShoot }, { Body::RifleFireballShootUp }, { Body::RifleFireballShootDown },
        { Body::RifleFireballShootUp }, { Body::RifleFireballShootDown },
        { Body::RifleCrouch }, { Body::RifleCrouchIdle }, { Body::RifleCrouchWalk }, { Body::RifleCrouchTurn }, { Body::RifleCrouchFireballShoot }, { Body::RifleCrouchAfterShoot },
        { Body::RifleLookUp, true }, { Body::RifleCrouch, true },
        { Body::DeathByStabbing }, { Body::DeathByGrenade },
    };

    static_assert(std::size(PISTOL_POSES) == (size_t)Pose::COUNT);
    static_assert(std::size(SPRAY_POSES) == (size_t)Pose::COUNT);
    static_assert(std::size(FIREBALL_POSES) == (size_t)Pose::COUNT);

    // indexed by WeaponType, the size is checked in animation.cpp
    constexpr const PoseAnim* POSE_ANIM[]{ PISTOL_POSES, SPRAY_POSES, FIREBALL_POSES };
}