#include "legs.hpp"
#include "body.hpp"

namespace player
{
    namespace
    {
        BodyState body_state(const LegsSM::Ctx& c) { return c.scene.get<State<BodyState>>(body_of(c)).current; }


        void stand_enter(const LegsSM::Ctx& c)
        {
            play(c.scene.get<Sprite>(c.entity), Legs::Idle);
            c.scene.get<Collider>(c.entity).velocity.x = 0.0f;

            if (c.state.previous == LegsState::Walk &&
                body_state(c) == BodyState::Ready)
                begin_transition(c.scene, body_of(c), Pose::WalkStop);
        }

        void walk_enter(const LegsSM::Ctx& c)
        {
            play(c.scene.get<Sprite>(c.entity), Legs::Walk);
        }

        void air_enter(const LegsSM::Ctx& c)
        {
            const Intent in = c.scene.get<Intent>(c.entity);

            // side jump only when the jump starts while moving, not for falling or steering later
            const bool side = (in.just_pressed & Intent::Jump) && in.dir.x != 0;
            play(c.scene.get<Sprite>(c.entity), side ? Legs::SideJump : Legs::InAir);

            const entity_id body = body_of(c);
            const Pose t = c.scene.get<BodyData>(body).transition;

            if (body_state(c) == BodyState::Transition &&
                t != Pose::LookUp && t != Pose::LookUpEnd)
                body_sm().force(c.scene, body, BodyState::Ready);
        }

        void crouch_enter(const LegsSM::Ctx& c) { begin_transition(c.scene, body_of(c), Pose::CrouchDown); }
        void crouch_exit(const LegsSM::Ctx& c) { begin_transition(c.scene, body_of(c), Pose::CrouchUp); }


        // what Stand, Walk and Crouch share
        LegsState grounded(const LegsSM::Ctx& c, const float speed, const bool can_jump = true)
        {
            const Intent in  = c.scene.get<Intent>(c.entity);
            Collider&    col = c.scene.get<Collider>(c.entity);

            col.velocity.x = (float)in.dir.x * speed;

            // facing only changes here: steering in the air is fine, turning around isn't
            if (LegsData& ld = c.scene.get<LegsData>(c.entity);
                in.dir.x != 0 && in.dir.x != ld.facing)
            {
                ld.facing = in.dir.x;

                Pose turn = Pose::Turn;
                if (c.state.current == LegsState::Crouch) turn = Pose::CrouchTurn;
                else if (in.dir.y < 0) turn = Pose::LookUpTurn;

                begin_transition(c.scene, body_of(c), turn);
            }

            if (!col.grounded) return LegsState::Air;

            // the impulse is here and not in air_enter, walking off a ledge enters Air too
            if (can_jump && (in.just_pressed & Intent::Jump))
            {
                col.velocity.y = -JUMP_VELOCITY;
                return LegsState::Air;
            }

            if (in.dir.y > 0) return LegsState::Crouch;
            return in.dir.x != 0 ? LegsState::Walk : LegsState::Stand;
        }

        LegsState stand(const LegsSM::Ctx& c) { return grounded(c, 0.0f); }
        LegsState walk(const LegsSM::Ctx& c) { return grounded(c, WALK_SPEED); }

        LegsState crouch(const LegsSM::Ctx& c)
        {
            const Intent in = c.scene.get<Intent>(c.entity);

            // no crawling or jumping while shooting, only turning and standing up
            const bool shooting = body_state(c) == BodyState::Shooting || (in.just_pressed & Intent::Fire);
            return grounded(c, shooting ? 0.0f : CRAWL_SPEED, !shooting);
        }

        LegsState air(const LegsSM::Ctx& c)
        {
            const Intent in  = c.scene.get<Intent>(c.entity);
            Collider&    col = c.scene.get<Collider>(c.entity);

            col.velocity.x = (float)in.dir.x * WALK_SPEED;

            if (!col.grounded)
            {
                // looking down doesn't go with the side jump legs
                if (Sprite& sprite = c.scene.get<Sprite>(c.entity);
                    in.dir.y > 0 && sprite.anim<Legs>() == Legs::SideJump)
                    play(sprite, Legs::InAir);

                return LegsState::Air;
            }

            if (in.dir.y > 0) return LegsState::Crouch;
            return in.dir.x ? LegsState::Walk : LegsState::Stand;
        }
    }


    const LegsSM& legs_sm()
    {
        static constexpr LegsSM sm
        {
            LegsSM::Behaviour{ stand_enter, stand, nullptr },       // Stand
            LegsSM::Behaviour{ walk_enter, walk, nullptr },         // Walk
            LegsSM::Behaviour{ air_enter, air, nullptr },           // Air
            LegsSM::Behaviour{ crouch_enter, crouch, crouch_exit }, // Crouch
        };

        return sm;
    }
}
