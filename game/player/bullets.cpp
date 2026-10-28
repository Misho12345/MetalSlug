#include "bullets.hpp"
#include "layer.hpp"
#include "projectile.hpp"

namespace ms
{
    using namespace mse;
    namespace
    {
        constexpr uint32_t POOL_SIZE = 32;

        constexpr uint8_t FIREBALL_CREATE_TICKS = 2;
        constexpr uint8_t FIREBALL_TICKS        = 2;
        constexpr uint8_t FIREBALL_EXPIRE_TICKS = 2;


        enum class BulletDir : uint8_t { Side, Up, Down, DiagUp, DiagDown };

        struct PlayerBullet final
        {
            WeaponType type;
            BulletDir  dir;
        };


        struct FireballAnims final
        {
            anim::Projectile create, loop, expire;
        };

        // indexed by BulletDir
        constexpr anim::Projectile PISTOL[]
        {
            anim::Projectile::Pistol,
            anim::Projectile::Pistol90deg, anim::Projectile::Pistol90deg,
            anim::Projectile::Pistol45deg, anim::Projectile::Pistol45deg,
        };

        // indexed by BulletDir
        constexpr FireballAnims FIREBALL[]
        {
            { anim::Projectile::RifleFireballCreate,     anim::Projectile::RifleFireball,     anim::Projectile::RifleFireballExpire     },
            { anim::Projectile::RifleFireballCreateUp,   anim::Projectile::RifleFireballUp,   anim::Projectile::RifleFireballExpireUp   },
            { anim::Projectile::RifleFireballCreateDown, anim::Projectile::RifleFireballDown, anim::Projectile::RifleFireballExpireDown },
        };

        // nearest of the three sprites, the in between bullets of a spray turn land on either side
        BulletDir dir_of(const vec2 velocity)
        {
            constexpr float TAN_22_5 = 0.4142f;

            const vec2 a  = glm::abs(velocity);
            const bool up = velocity.y < 0.0f;

            if (a.y <= a.x * TAN_22_5) return BulletDir::Side;
            if (a.x <= a.y * TAN_22_5) return up ? BulletDir::Up : BulletDir::Down;
            return up ? BulletDir::DiagUp : BulletDir::DiagDown;
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
            // over the front water, crouching in it the gun is under the surface
            Sprite& spr = scene.set<Sprite>(bullet, anim::Projectile::Pistol);
            spr.hidden  = true;
            spr.layer   = 3;
            scene.set<PlayerBullet>(bullet, WeaponType::Pistol);
            scene.set<Projectile>(bullet, ProjectileType::Bullet);

            Collider& col    = scene.set<Collider>(bullet);
            col.enabled      = false;
            col.gravity      = 0.0f;
            col.target_layer |= Layer::TileMap | Layer::POW | Layer::Enemy | Layer::Crate;
            col.layer        |= Layer::PlayerProjectile;

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
        scene.get<Projectile>(bullet).type = type == WeaponType::RifleFireball ? ProjectileType::Fireball : ProjectileType::Bullet;

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
                spr.play(PISTOL[(size_t)b.dir]);
                break;


            case WeaponType::RifleFireball:
                assert(b.dir <= BulletDir::Down);
                spr.play(FIREBALL[(size_t)b.dir].create, FIREBALL_CREATE_TICKS);
                break;

            default: assert(!"not implemented");
        }

        tr.position  = from;

        col.velocity = velocity;
        col.size = spr.size();

        // the 45 sprite points up-right, mirrored it's down-right
        spr.flip_x = (facing < 0) != (b.dir == BulletDir::DiagDown);
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
                const anim::Projectile   playing = spr.anim<anim::Projectile>();

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
