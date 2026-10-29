#pragma once
#include "mse/mse.hpp"
#include "enemy.hpp"

namespace ms
{
    using RebelAnim = mse::anim::enemy::Rebel;

    constexpr EnemyAnimData data(const RebelAnim a)
    {
        switch (a)
        {
            case RebelAnim::Idle1: return { 6, { -2, -20 } };
            case RebelAnim::Idle2: return { 6, { -1, -20 } };
            case RebelAnim::Idle3: return { 6, { -3, -19 } };

            case RebelAnim::Sneaking:    return { 5, { -8, -19 }, true };
            case RebelAnim::Running:     return { 3, { -3, -20 }, true };
            case RebelAnim::RunningAway: return { 3, { -7, -20 }, true };

            case RebelAnim::Turning:    return { 3, {  0, -20 } };
            case RebelAnim::QuickTurn:  return { 4, { -1, -19 } };
            case RebelAnim::ScaredTurn: return { 4, { -1, -19 } };
            case RebelAnim::Scared:     return { 4, { -4, -22 } };
            case RebelAnim::Stopping:   return { 3, { -3, -19 } };

            case RebelAnim::Falling: return { 4, {  0, -22 } };
            case RebelAnim::Jumping: return { 3, { -6, -20 } };

            case RebelAnim::KnifeSlash:      return { 3, { -12, -19 } };
            case RebelAnim::ThrowingGrenade: return { 4, {  -3, -21 } };

            case RebelAnim::Death1:       return { 4, {  2, -19 } };
            case RebelAnim::Death2:       return { 4, {  1, -20 } };
            case RebelAnim::Death3:       return { 4, {  0, -20 } };
            case RebelAnim::Death4:       return { 4, {  8, -17 } };
            case RebelAnim::Death5:       return { 4, {  8, -20 } };
            case RebelAnim::Death6:       return { 4, { 12, -22 } };
            case RebelAnim::BloodyDeath1: return { 4, {  8, -25 } };
            case RebelAnim::BloodyDeath2: return { 4, {  7, -23 } };

            case RebelAnim::OnFireBig:    return { 3, { 2, -32 }, true };
            case RebelAnim::DeathFireBig: return { 4, { 6, -32 } };

            default: assert(!"not implemented"); return {};
        }
    }
}
