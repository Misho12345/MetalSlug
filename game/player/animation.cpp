#include "animation.hpp"

namespace ms
{
    namespace
    {
        // legs collider height when hidden (for tilemap)
        constexpr uint32_t LEGS_SMALL_HEIGHT = 4;

        // keeps the bottom of the small box where the bottom of the full one is
        constexpr int32_t LEGS_SMALL_OFFSET = (int32_t)(LEGS_SIZE.y / 2) - (int32_t)(LEGS_SMALL_HEIGHT / 2);

        static_assert(std::size(POSE_ANIM) == (size_t)WeaponType::COUNT);


        Pose pick_pose(
            const BodyState         body,
            const State<LegsState>& legs,
            const Intent            in,
            const bool              side_jump,
            const BodyData&         bd)
        {
            if (body == BodyState::Dead) return bd.death;
            if (body == BodyState::Transition) return bd.transition;

            const bool crouching = legs.current == LegsState::Crouch;
            const bool in_air    = legs.current == LegsState::Air;

            if (body == BodyState::Shooting)
            {
                if (crouching) return Pose::CrouchShoot;
                if (bd.aim == Aim::Up) return Pose::ShootUp;
                if (bd.aim == Aim::Down) return Pose::ShootDown;
                if (bd.aim == Aim::DiagUp) return Pose::ShootDiagUp;
                if (bd.aim == Aim::DiagDown) return Pose::ShootDiagDown;
                return in_air ? Pose::JumpShoot : Pose::Shoot;
            }

            if (crouching)
            {
                if (in.dir.x != 0) return Pose::CrouchWalk;

                if (bd.last_pose == Pose::CrouchShoot ||
                    bd.last_pose == Pose::CrouchAfterShoot)
                    return Pose::CrouchAfterShoot;

                return Pose::CrouchIdle;
            }

            if (in.dir.y < 0) return Pose::LookUpIdle;

            if (in_air)
            {
                if (in.dir.y > 0) return Pose::LookDown;
                return side_jump ? Pose::SideJump : Pose::InAir;
            }

            return legs.current == LegsState::Walk ? Pose::Walk : Pose::Idle;
        }

        ivec2 sprite_center(const ivec2 box_center, const ivec2 origin, const ivec2 size, const bool flip)
        {
            const ivec2 box_min = box_center - ivec2(LEGS_SIZE / 2u);
            const ivec2 box_max = box_min + ivec2(LEGS_SIZE);

            // mirrored around the box when facing left
            const int32_t left = flip ? box_max.x - origin.x - size.x : box_min.x + origin.x;

            return ivec2{ left, box_min.y + origin.y } + size / 2;
        }
    }


    void animate_body(Scene& scene, const entity_id legs, const entity_id body)
    {
        Sprite&   body_spr = scene.get<Sprite>(body);
        BodyData& bd       = scene.get<BodyData>(body);

        const Pose pose = pick_pose(
            scene.get<State<BodyState>>(body).current,
            scene.get<State<LegsState>>(legs),
            scene.get<Intent>(legs),
            scene.get<Sprite>(legs).anim<Legs>() == Legs::SideJump,
            bd);

        const PoseAnim wanted = POSE_ANIM[(size_t)bd.weapon][(size_t)pose];

        if (wanted != bd.requested || bd.restart)
        {
            // a crouch shot has already chained into its after shoot when it ends, that one just stays
            if (bd.restart || wanted.reversed ||
                body_spr.anim<Body>() != wanted.anim)
                play(body_spr, wanted.anim, wanted.reversed);

            bd.requested = wanted;
            bd.restart   = false;
        }
        else if (const BodyAnim playing = data(body_spr.anim<Body>());
            body_spr.anim_done() && playing.end == End::Next)
            play(body_spr, playing.next);

        bd.last_pose = pose;
    }

    ivec2 place_sprites(Scene& scene, const entity_id legs, const entity_id body)
    {
        Transform& legs_tr  = scene.get<Transform>(legs);
        Sprite&    legs_spr = scene.get<Sprite>(legs);
        Collider&  legs_col = scene.get<Collider>(legs);
        LegsData&  ld       = scene.get<LegsData>(legs);

        Transform& body_tr  = scene.get<Transform>(body);
        Sprite&    body_spr = scene.get<Sprite>(body);
        Collider&  body_col = scene.get<Collider>(body);

        const BodyAnim body_data = data(body_spr.anim<Body>());
        const LegsAnim legs_data = data(legs_spr.anim<Legs>());
        const bool     full      = body_data.full_body;

        const bool flip = ld.facing < 0;
        legs_spr.flip_x = body_spr.flip_x = flip;
        legs_spr.hidden = full;

        const ivec2 center = legs_tr.position - ld.sprite_offset;

        const ivec2 legs_size = legs_spr.size() * legs_tr.scale;
        const ivec2 body_size = body_spr.size() * body_tr.scale;

        legs_tr.position = sprite_center(center, legs_data.origin, legs_size, flip);
        ld.sprite_offset = legs_tr.position - center;

        legs_col.offset = -ld.sprite_offset;
        legs_col.size.y = full ? LEGS_SMALL_HEIGHT : LEGS_SIZE.y;
        if (full) legs_col.offset.y += LEGS_SMALL_OFFSET;

        ivec2 body_origin = body_data.origin;
        if (!full) body_origin.y -= legs_data.waist_lift;

        body_tr.position = sprite_center(center, body_origin, body_size, flip);
        body_col.size    = body_size;

        return center;
    }
}
