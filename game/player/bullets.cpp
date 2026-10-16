#include "bullets.hpp"
#include "layer.hpp"

namespace player
{
    namespace
    {
        constexpr uint32_t POOL_SIZE = 32;

        constexpr uint8_t FIREBALL_CREATE_TICKS = 2;
        constexpr uint8_t FIREBALL_TICKS        = 2;
        constexpr uint8_t FIREBALL_EXPIRE_TICKS = 2;

        struct PlayerBullet final
        {
            WeaponType type;
        };

        // there's one player, and one bullet pool so it's fine really
        uint32_t next_bullet = 0;


        void stop_bullet(Scene& scene, const entity_id bullet, const WeaponType type)
        {
            Collider& col = scene.get<Collider>(bullet);
            col.enabled   = false;
            col.velocity  = vec2{ 0.0f };

            Sprite& spr = scene.get<Sprite>(bullet);

            if (type != WeaponType::RifleFireball) spr.hidden = true;
            else spr.play(anim::Bullet::RifleFireballExpire, FIREBALL_EXPIRE_TICKS);
        }

        entity_id take_bullet(Scene& scene)
        {
            const vector<entity_id>& bullets = scene.pool<PlayerBullet>().owners();

            for (uint32_t i = 0; i < POOL_SIZE; ++i)
            {
                const uint32_t slot = (next_bullet + i) % POOL_SIZE;

                if (scene.get<Sprite>(bullets[slot]).hidden)
                {
                    next_bullet = (slot + 1) % POOL_SIZE;
                    return bullets[slot];
                }
            }

            const entity_id e = bullets[next_bullet];
            next_bullet       = (next_bullet + 1) % POOL_SIZE;
            return e;
        }
    }


    void create_bullet_pool(Scene& scene)
    {
        for (uint32_t i = 0; i < POOL_SIZE; ++i)
        {
            const entity_id bullet = scene.create_entity();

            scene.set<Transform>(bullet);
            scene.set<Sprite>(bullet, anim::Bullet::Pistol, should_loop::no).hidden = true;
            scene.set<PlayerBullet>(bullet, WeaponType::Pistol);

            Collider& col    = scene.set<Collider>(bullet);
            col.enabled      = false;
            col.gravity      = 0.0f;
            col.target_layer |= Layer::TileMap | Layer::POW;
            col.layer        |= Layer::PlayerBullet;

            col.callback = +[](Scene& scene, const entity_id self, entity_id, const uint32_t layer)
            {
                if (const WeaponType type = scene.get<PlayerBullet>(self).type;
                    layer & Layer::TileMap || type != WeaponType::RifleFireball)
                    stop_bullet(scene, self, type);
            };
        }
    }

    void fire_bullet(Scene& scene, const WeaponType type, const ivec2 from, const vec2 velocity)
    {
        const entity_id bullet = take_bullet(scene);
        scene.get<PlayerBullet>(bullet).type = type;

        Transform& tr  = scene.get<Transform>(bullet);
        Sprite&    spr = scene.get<Sprite>(bullet);
        Collider&  col = scene.get<Collider>(bullet);

        spr.hidden  = false;
        col.enabled = true;

        switch (type)
        {
            case WeaponType::RifleSpray:
                [[fallthrough]];
                // TODO: find spray bullet, pistol bullet is fine for now ig

            case WeaponType::Pistol:
                spr.play(anim::Bullet::Pistol);
                break;

            case WeaponType::RifleFireball:
                spr.play(anim::Bullet::RifleFireballCreate, FIREBALL_CREATE_TICKS);
                break;

            default: assert(!"not implemented");
        }

        tr.position  = from;
        col.velocity = velocity;

        // Left is a flip and not a half turn, otherwise the fireball would be upside down
        // TODO: make exported perpendicular (and 45deg for spray) sprites for bullets to have meaningful pp checks
        spr.flip_x  = velocity.x < 0.0f;
        tr.rotation = glm::atan(velocity.y, glm::abs(velocity.x));
        if (spr.flip_x) tr.rotation = -tr.rotation;
    }

    void update_bullets(Scene& scene)
    {
        for (const entity_id bullet : scene.pool<PlayerBullet>().owners())
        {
            Sprite& spr = scene.get<Sprite>(bullet);
            if (spr.hidden) continue;

            // i could make a state machine for bullets, but it probably is overkill
            // maybe if they have more logic later or something and their state get's more complex
            if (scene.get<PlayerBullet>(bullet).type == WeaponType::RifleFireball)
            {
                const anim::Bullet playing = spr.anim<anim::Bullet>();

                if (playing == anim::Bullet::RifleFireballExpire)
                {
                    if ((spr.hidden = spr.anim_done())) spr.stop();
                    continue;
                }

                if (playing == anim::Bullet::RifleFireballCreate && spr.anim_done())
                {
                    spr.play(anim::Bullet::RifleFireball, FIREBALL_TICKS, is_reversed::no, should_loop::yes);
                }
            }

            const Transform& tr = scene.get<Transform>(bullet);
            if (spr.screen_bounds(tr, scene.camera_pos_screen()) & aabb::screen) continue;

            scene.get<Collider>(bullet).enabled = false;

            spr.hidden = true;
            spr.stop();
        }
    }
}
