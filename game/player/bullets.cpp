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


        enum class BulletDir : uint8_t { Side, Up, Down };

        struct PlayerBullet final
        {
            WeaponType type;
            BulletDir  dir;
        };


        struct FireballAnims final
        {
            anim::Bullet create, loop, expire;
        };

        // indexed by BulletDir
        constexpr FireballAnims FIREBALL[]
        {
            { anim::Bullet::RifleFireballCreate,     anim::Bullet::RifleFireball,     anim::Bullet::RifleFireballExpire     },
            { anim::Bullet::RifleFireballCreateUp,   anim::Bullet::RifleFireballUp,   anim::Bullet::RifleFireballExpireUp   },
            { anim::Bullet::RifleFireballCreateDown, anim::Bullet::RifleFireballDown, anim::Bullet::RifleFireballExpireDown },
        };

        BulletDir dir_of(const vec2 velocity)
        {
            if (glm::abs(velocity.x) >= glm::abs(velocity.y)) return BulletDir::Side;
            return velocity.y < 0.0f ? BulletDir::Up : BulletDir::Down;
        }



        // there's one player, and one bullet pool so it's fine really
        uint32_t next_bullet = 0;


        void stop_bullet(Scene& scene, const entity_id bullet, const PlayerBullet b)
        {
            Collider& col = scene.get<Collider>(bullet);
            col.enabled   = false;
            col.velocity  = vec2{ 0.0f };

            Sprite& spr = scene.get<Sprite>(bullet);

            if (b.type != WeaponType::RifleFireball) spr.hidden = true;
            else spr.play(FIREBALL[(size_t)b.dir].expire, FIREBALL_EXPIRE_TICKS);
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
                if (const PlayerBullet bul = scene.get<PlayerBullet>(self);
                    layer & Layer::TileMap || bul.type != WeaponType::RifleFireball)
                    stop_bullet(scene, self, bul);
            };
        }
    }

    void fire_bullet(Scene& scene, const WeaponType type, const ivec2 from, const vec2 velocity, const int8_t facing)
    {
        const entity_id bullet = take_bullet(scene);

        PlayerBullet& b = scene.set<PlayerBullet>(bullet, type, dir_of(velocity));

        Transform& tr  = scene.get<Transform>(bullet);
        Sprite&    spr = scene.get<Sprite>(bullet);
        Collider&  col = scene.get<Collider>(bullet);

        spr.hidden  = false;
        col.enabled = true;

        switch (type)
        {
            case WeaponType::RifleSpray:
                [[fallthrough]];
                // TODO: find spray bullet (+45 deg), pistol bullet is fine for now ig

            case WeaponType::Pistol:
                spr.play(b.dir == BulletDir::Side ? anim::Bullet::Pistol : anim::Bullet::Pistol90deg);
                break;


            case WeaponType::RifleFireball:
                spr.play(FIREBALL[(size_t)b.dir].create, FIREBALL_CREATE_TICKS);
                break;

            default: assert(!"not implemented");
        }

        tr.position  = from;
        col.velocity = velocity;
        spr.flip_x = facing < 0;
    }

    void update_bullets(Scene& scene)
    {
        for (const entity_id bullet : scene.pool<PlayerBullet>().owners())
        {
            Sprite& spr = scene.get<Sprite>(bullet);
            if (spr.hidden) continue;

            if (scene.get<PlayerBullet>(bullet).type == WeaponType::RifleFireball)
            {
                const FireballAnims& fa      = FIREBALL[(size_t)scene.get<PlayerBullet>(bullet).dir];
                const anim::Bullet   playing = spr.anim<anim::Bullet>();

                if (playing == fa.expire)
                {
                    if ((spr.hidden = spr.anim_done())) spr.stop();
                    continue;
                }

                if (playing == fa.create && spr.anim_done())
                {
                    spr.play(fa.loop, FIREBALL_TICKS, is_reversed::no, should_loop::yes);
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
