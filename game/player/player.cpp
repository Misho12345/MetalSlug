#include "player.hpp"
#include "legs.hpp"
#include "body.hpp"
#include "bullets.hpp"
#include "animation.hpp"
#include "layer.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        constexpr ivec2 CAMERA_OFFSET{ 0, -22 };

        constexpr uint16_t BLINK_TICKS = 2;
    }


    void Player::register_machines() const
    {
        // legs must be first, the body reads their state
        StateMachineSystem& sms = App::ctx().state_machines;
        sms.add(legs_sm());
        sms.add(body_sm());
    }


    void Player::create(Scene& scene)
    {
        legs_  = scene.create_entity();
        body_  = scene.create_entity();
        score_ = 0;

        scene.set<Transform>(legs_, ivec2{ 100, -50 });
        scene.set<Transform>(body_);

        scene.set<Sprite>(legs_, Legs::Idle, should_loop::yes);
        scene.set<Sprite>(body_, Body::PistolIdle, should_loop::yes).layer = 1;


        const Auto callback = [](
            Scene&          scene, const entity_id self,
            const entity_id other, const uint32_t  layer)
        {
            BodyData& bd = scene.get<BodyData>(scene.has<BodyData>(self)
                                                   ? self
                                                   : scene.get<LegsData>(self).body);

            if (layer & Layer::WeaponUpgrade)
            {
                WeaponUpgrade& u = scene.get<WeaponUpgrade>(other);
                if (u.claimed) return;
                u.claimed = true;

                bd.weapon = u.type;
                bd.ammo = WEAPONS[(size_t)bd.weapon].init_ammo;
            }

            if ((layer & (Layer::EnemyKnife | Layer::EnemyProjectile)) &&
                bd.death == Pose::COUNT && bd.invulnerable == 0)
                bd.death = layer & Layer::EnemyKnife ? Pose::DeathByStabbing : Pose::DeathByGrenade;
        };

        Collider& legs_col = scene.set<Collider>(legs_);

        legs_col.layer |= Layer::Player;
        legs_col.target_layer |= Layer::WeaponUpgrade | Layer::EnemyKnife | Layer::EnemyProjectile;

        legs_col.gravity = JUMP_GRAVITY;
        legs_col.size    = LEGS_SIZE;

        legs_col.callback = callback;


        Collider& body_col = scene.set<Collider>(body_);

        body_col.layer |= Layer::Player;
        body_col.target_layer |= Layer::WeaponUpgrade | Layer::EnemyKnife | Layer::EnemyProjectile;

        body_col.gravity               = 0.0f;
        body_col.collide_with_tile_map = false;

        body_col.callback = callback;


        scene.set<Intent>(legs_);
        scene.set<LegsData>(legs_, body_);
        scene.set<BodyData>(body_, legs_);

        create_bullet_pool(scene);

        legs_sm().start(scene, legs_, LegsState::Stand);
        body_sm().start(scene, body_, BodyState::Ready);
    }


    void Player::update(Scene& scene) const
    {
        Intent& in = scene.set<Intent>(legs_); // resets the intent
        if (!alive(scene)) return;

        if (Input::down(Key::A)) --in.dir.x;
        if (Input::down(Key::D)) ++in.dir.x;
        if (Input::down(Key::W)) --in.dir.y;
        if (Input::down(Key::S)) ++in.dir.y;

        if (Input::down(Key::Space)) in.down |= Intent::Jump;
        if (Input::down(Key::J)) in.down |= Intent::Fire;

        if (Input::just_pressed(Key::Space)) in.just_pressed |= Intent::Jump;
        if (Input::just_pressed(Key::J)) in.just_pressed |= Intent::Fire;
    }

    void Player::late_update(Scene& scene) const
    {
        animate_body(scene, legs_, body_);
        const ivec2 center = place_sprites(scene, legs_, body_);

        BodyData& bd = scene.get<BodyData>(body_);
        if (bd.invulnerable > 0) --bd.invulnerable;

        const bool blink = (bd.invulnerable / BLINK_TICKS) % 2 != 0;
        scene.get<Sprite>(legs_).color.a = scene.get<Sprite>(body_).color.a = (uint8_t)(blink ? 0 : 255);

        ivec2 off = center + CAMERA_OFFSET;

        // clamp camera to bg sprite
        const aabb bg = scene.get<Sprite>(scene.bg_entity).bounds(scene.get<Transform>(scene.bg_entity));
        const aabb sc = aabb::screen + (off - HALF_RESOLUTION_i);

        if (sc.min.x < bg.min.x) off.x += bg.min.x - sc.min.x;
        if (sc.min.y < bg.min.y) off.y += bg.min.y - sc.min.y;

        if (sc.max.x > bg.max.x) off.x -= sc.max.x - bg.max.x;
        if (sc.max.y > bg.max.y) off.y -= sc.max.y - bg.max.y;

        scene.get<Transform>(scene.camera()).position = off;

        update_bullets(scene);
    }


    bool Player::alive(const Scene& scene) const { return scene.get<State<BodyState>>(body_).current != BodyState::Dead; }
    bool Player::shooting(const Scene& scene) const { return scene.get<State<BodyState>>(body_).current == BodyState::Shooting; }

    int8_t Player::facing(const Scene& scene) const { return scene.get<LegsData>(legs_).facing; }

    bool Player::out_of_lives(const Scene& scene) const
    {
        return scene.get<BodyData>(body_).lives == 0 &&
               scene.get<State<BodyState>>(body_).ticks >= DEAD_TICKS;
    }
}
