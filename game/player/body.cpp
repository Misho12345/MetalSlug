#include "body.hpp"
#include "bullets.hpp"

namespace player
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
                default: assert(!"not implemented");
            }

            return {};
        }

        void fire(const BodySM::Ctx& c)
        {
            const entity_id l   = legs_of(c);
            const Intent    in  = c.scene.get<Intent>(l);
            const bool      air = c.scene.get<State<LegsState>>(l).current == LegsState::Air;

            BodyData&       bd = c.scene.get<BodyData>(c.entity);
            const LegsData& ld = c.scene.get<LegsData>(l);

            // locked here so the bullet and the animation agree if up is released halfway
            if (bd.burst_shot == 0)
            {
                if (in.dir.y > 0) bd.aim = air ? Aim::Down : Aim::Forward;
                else bd.aim = in.dir.y < 0 ? Aim::Up : Aim::Forward;
            }

            // landed mid burst
            if (bd.aim == Aim::Down && !air) bd.aim = Aim::Forward;

            bd.restart = true;

            const ivec2 dir = aim_dir(bd.aim, ld.facing);
            vec2 vel = vec2(dir) * WEAPONS[(size_t)bd.weapon].speed;

            if (bd.weapon == WeaponType::RifleSpray)
            {
                // pick angle from SPRAY_SPREAD
                vel = glm::rotate(vel, SPRAY_SPREAD[(size_t)bd.burst_shot % std::size(SPRAY_SPREAD)]);
            }

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

            fire(c);
            c.state.ticks = 0;
        }


        BodyState on_ready(const BodySM::Ctx& c)
        {
            note_fire(c);
            if (look_changed(c)) return BodyState::Transition;

            return c.scene.get<BodyData>(c.entity).fire_queued ? BodyState::Shooting : BodyState::Ready;
        }

        BodyState on_shooting(const BodySM::Ctx& c)
        {
            note_fire(c);

            BodyData&         bd  = c.scene.get<BodyData>(c.entity);
            const WeaponData& gun = WEAPONS[(size_t)bd.weapon];

            if (bd.burst_left == 0 && look_changed(c)) return BodyState::Transition;

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

            return !anim_pending(c) && shot_done(c) ? BodyState::Ready : BodyState::Shooting;
        }

        // Turning and changing level (crouch <-> stand <-> looking up) have their own sprites.
        // No action can be taken (firing) while the sprite is in this state
        BodyState on_transition(const BodySM::Ctx& c)
        {
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
    }


    const BodySM& body_sm()
    {
        using B = BodySM::Behaviour;
        static constexpr BodySM sm
        {
            B{ nullptr, on_ready, nullptr },        // Ready
            B{ start_burst, on_shooting, nullptr }, // Shooting
            B{ nullptr, on_transition, nullptr },   // Transition
        };

        return sm;
    }

    void begin_transition(Scene& scene, const entity_id body, const Pose pose)
    {
        BodyData& bd  = scene.get<BodyData>(body);
        bd.transition = pose;
        bd.restart    = true;

        body_sm().force(scene, body, BodyState::Transition);
        scene.get<State<BodyState>>(body).ticks = 0;
    }
}
