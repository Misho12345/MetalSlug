#include "body.hpp"
#include "bullets.hpp"
#include "metal_slug.hpp"

namespace ms
{
    namespace
    {
        constexpr int32_t SHOOT_OFFSET = 10;


        // raising and lowering the gun. Not while crouching, down is held then
        bool look_changed(const BodySM::Ctx& c)
        {
            const entity_id l  = legs_of(c);
            const bool      up = c.scene.get<Intent>(l).dir.y < 0;

            BodyData& bd = c.scene.get<BodyData>(c.entity);
            if (up == bd.was_up) return false;

            bd.was_up = up;
            if (c.scene.get<State<LegsState>>(l).current == LegsState::Crouch) return false;

            bd.transition = up ? Pose::LookUp : Pose::LookUpEnd;
            bd.restart    = true;
            return true;
        }

        void note_fire(const BodySM::Ctx& c)
        {
            if (c.scene.get<Intent>(legs_of(c)).just_pressed & Intent::Fire)
            {
                c.scene.get<BodyData>(c.entity).fire_queued = true;
            }
        }

        bool killed(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).death != Pose::COUNT; }

        // true while the animation that was asked for hasn't been played yet (late_update does it, after the machines)
        bool anim_pending(const BodySM::Ctx& c) { return c.scene.get<BodyData>(c.entity).restart; }

        // crouch shoot chains into after shoot, so the shot isn't over when the first one is
        bool shot_done(const BodySM::Ctx& c)
        {
            const Sprite&   sprite  = c.scene.get<Sprite>(c.entity);
            const BodyData& bd      = c.scene.get<BodyData>(c.entity);
            const Body      playing = sprite.anim<Body>();

            if (playing == POSE_ANIM[(size_t)bd.weapon][(size_t)Pose::CrouchAfterShoot].anim) return true;
            return sprite.anim_done() && data(playing).end != End::Next;
        }

        ivec2 aim_dir(const Aim aim, const int8_t facing)
        {
            switch (aim)
            {
                case Aim::Forward: return ivec2{ facing, 0 };
                case Aim::Up: return ivec2{ 0, -1 };
                case Aim::Down: return ivec2{ 0, 1 };
                case Aim::DiagUp: return ivec2{ facing, -1 };
                case Aim::DiagDown: return ivec2{ facing, 1 };
                default: assert(!"not implemented");
            }

            return {};
        }

        bool in_air(const BodySM::Ctx& c) { return c.scene.get<State<LegsState>>(legs_of(c)).current == LegsState::Air; }

        Aim held_aim(const BodySM::Ctx& c)
        {
            const Intent in = c.scene.get<Intent>(legs_of(c));

            if (in.dir.y < 0) return Aim::Up;
            return in.dir.y > 0 && in_air(c) ? Aim::Down : Aim::Forward;
        }

        int32_t held_tilt(const BodySM::Ctx& c)
        {
            const Aim aim = held_aim(c);

            if (aim == Aim::Up) return -SPRAY_SWEEP_STEPS;
            return aim == Aim::Down ? SPRAY_SWEEP_STEPS : 0;
        }

        // nearest of the three poses, split at 22.5 and 67.5 like the bullet sprites
        Aim tilt_aim(const int8_t tilt)
        {
            const int32_t t  = glm::abs((int32_t)tilt);
            const bool    up = tilt < 0;

            if (4 * t < SPRAY_SWEEP_STEPS) return Aim::Forward;
            if (4 * t > 3 * SPRAY_SWEEP_STEPS) return up ? Aim::Up : Aim::Down;
            return up ? Aim::DiagUp : Aim::DiagDown;
        }

        // per bullet rather than per burst, so a turn can start mid burst and carry over into the next one
        vec2 spray(BodyData& bd, const int8_t facing, const bool air, const int32_t to)
        {
            // landed mid burst
            if (!air && bd.tilt > 0) bd.tilt = 0;

            const float angle   = (float)bd.tilt * SPRAY_SWEEP_ANGLE;
            const bool  turning = bd.tilt != to;

            vec2 vel = vec2{ (float)facing * glm::cos(angle), glm::sin(angle) } * WEAPONS[(size_t)WeaponType::RifleSpray].speed;

            // the spread would bunch up the turn's bullets
            if (!turning) vel = glm::rotate(vel, SPRAY_SPREAD[(size_t)bd.burst_shot % std::size(SPRAY_SPREAD)]);

            bd.aim    = tilt_aim(bd.tilt);
            bd.was_up = bd.aim == Aim::Up;

            // stepped after the shot, so a turn always starts with one more bullet the old way
            if (turning) bd.tilt = (int8_t)(bd.tilt + (bd.tilt < to ? 1 : -1));

            return vel;
        }

        void fire(const BodySM::Ctx& c)
        {
            const bool air = in_air(c);

            BodyData&       bd = c.scene.get<BodyData>(c.entity);
            const LegsData& ld = c.scene.get<LegsData>(legs_of(c));

            vec2 vel;
            if (bd.weapon == WeaponType::RifleSpray) vel = spray(bd, ld.facing, air, held_tilt(c));
            else
            {
                // locked here so the bullet and the animation agree if up is released halfway
                if (bd.burst_shot == 0) bd.aim = held_aim(c);

                // landed mid burst
                if (bd.aim == Aim::Down && !air) bd.aim = Aim::Forward;

                vel = vec2(aim_dir(bd.aim, ld.facing)) * WEAPONS[(size_t)bd.weapon].speed;
            }

            bd.restart = true;

            const ivec2 dir = aim_dir(bd.aim, ld.facing);
            fire_bullet(c.scene, bd.weapon, c.scene.get<Transform>(c.entity).position + dir * SHOOT_OFFSET, vel, ld.facing);

            ++bd.burst_shot;
            --bd.burst_left;

            if (bd.weapon != WeaponType::Pistol && !--bd.ammo) bd.weapon = WeaponType::Pistol;
        }

        void start_burst(const BodySM::Ctx& c)
        {
            BodyData& bd = c.scene.get<BodyData>(c.entity);

            bd.fire_queued = false;
            bd.burst_left  = WEAPONS[(size_t)bd.weapon].burst;
            bd.burst_shot  = 0;

            // once per burst, the machine gun's sound already is a whole burst
            constexpr Sfx SHOT_SFX[]{ Sfx::Pistol, Sfx::MachineGun, Sfx::FireShot }; // indexed by WeaponType
            static_assert(std::size(SHOT_SFX) == (size_t)WeaponType::COUNT);
            MetalSlug::user_ctx().sounds.play(SHOT_SFX[(size_t)bd.weapon]);

            fire(c);
            c.state.ticks = 0;
        }

        // coming from another state there's nothing to sweep from
        void shooting_enter(const BodySM::Ctx& c)
        {
            BodyData& bd = c.scene.get<BodyData>(c.entity);
            bd.aim  = held_aim(c);
            bd.tilt = (int8_t)held_tilt(c);

            start_burst(c);
        }


        BodyState on_ready(const BodySM::Ctx& c)
        {
            if (killed(c)) return BodyState::Dead;
            note_fire(c);
            if (look_changed(c)) return BodyState::Transition;

            return c.scene.get<BodyData>(c.entity).fire_queued ? BodyState::Shooting : BodyState::Ready;
        }

        BodyState on_shooting(const BodySM::Ctx& c)
        {
            if (killed(c)) return BodyState::Dead;
            note_fire(c);

            BodyData&         bd  = c.scene.get<BodyData>(c.entity);
            const WeaponData& gun = WEAPONS[(size_t)bd.weapon];

            // the spray turns by sweeping (fire) instead of leaving Shooting
            if (bd.weapon != WeaponType::RifleSpray && bd.burst_left == 0 && look_changed(c)) return BodyState::Transition;

            if (bd.fire_queued && c.state.ticks >= gun.refire_ticks)
            {
                start_burst(c);
                return BodyState::Shooting;
            }

            if (bd.burst_left > 0)
            {
                if (c.state.ticks >= gun.burst_ticks)
                {
                    fire(c);
                    c.state.ticks = 0;
                }

                return BodyState::Shooting;
            }

            if (anim_pending(c) || !shot_done(c)) return BodyState::Shooting;

            // the spray stopped firing mid turn, the gun still has to get there
            return look_changed(c) ? BodyState::Transition : BodyState::Ready;
        }

        // Turning and changing level (crouch <-> stand <-> looking up) have their own sprites.
        // No action can be taken (firing) while the sprite is in this state
        BodyState on_transition(const BodySM::Ctx& c)
        {
            if (killed(c)) return BodyState::Dead;
            note_fire(c);

            const entity_id l  = legs_of(c);
            const BodyData& bd = c.scene.get<BodyData>(c.entity);

            // the only one that can be interrupted
            if (bd.transition == Pose::WalkStop)
            {
                if (c.scene.get<State<LegsState>>(l).current != LegsState::Stand) return BodyState::Ready;
                if (bd.fire_queued) return BodyState::Shooting;
            }

            if (anim_pending(c) || !c.scene.get<Sprite>(c.entity).anim_done()) return BodyState::Transition;

            return bd.fire_queued ? BodyState::Shooting : BodyState::Ready;
        }


        void dead_enter(const BodySM::Ctx& c)
        {
            BodyData& bd = c.scene.get<BodyData>(c.entity);
            bd.restart   = true;

            --bd.lives;
        }

        // after the last life he stays down, the game over comes from MetalSlug::late_update
        BodyState on_dead(const BodySM::Ctx& c)
        {
            return c.state.ticks >= DEAD_TICKS && c.scene.get<BodyData>(c.entity).lives > 0
                       ? BodyState::Ready
                       : BodyState::Dead;
        }

        // respawning in place
        void dead_exit(const BodySM::Ctx& c)
        {
            BodyData& bd = c.scene.get<BodyData>(c.entity);

            bd.death        = Pose::COUNT;
            bd.invulnerable = INVULNERABLE_TICKS;
            bd.weapon       = WeaponType::Pistol;
            bd.ammo         = 0;
            bd.fire_queued  = false;
            bd.burst_left   = 0;
            bd.restart      = true;
        }
    }


    const BodySM& body_sm()
    {
        using B = BodySM::Behaviour;
        static constexpr BodySM sm
        {
            B{ nullptr, on_ready, nullptr },           // Ready
            B{ shooting_enter, on_shooting, nullptr }, // Shooting
            B{ nullptr, on_transition, nullptr },      // Transition
            B{ dead_enter, on_dead, dead_exit },       // Dead
        };

        return sm;
    }

    void begin_transition(Scene& scene, const entity_id body, const Pose pose)
    {
        if (scene.get<State<BodyState>>(body).current == BodyState::Dead) return;

        BodyData& bd  = scene.get<BodyData>(body);
        bd.transition = pose;
        bd.restart    = true;

        body_sm().force(scene, body, BodyState::Transition);
        scene.get<State<BodyState>>(body).ticks = 0;
    }
}
