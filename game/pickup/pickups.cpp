#include "pickups.hpp"
#include "layer.hpp"
#include "spawn.hpp"
#include "metal_slug.hpp"

#include "player/components.hpp"
#include "hud/number.hpp"

namespace ms
{
    using namespace mse;

    namespace
    {
        constexpr uint8_t CRATE_TICKS[]{ 70, 4, 4, 4, 4, 4 };

        constexpr uint8_t EXPLOSION_TICKS = 2;
        constexpr int32_t EXPLOSION_LAYER = 3;

        constexpr int32_t CRATE_LAYER = -1; // behind the player
        constexpr int32_t DROP_LAYER  = 3;  // over the front water, on the riverbed the points would be under it

        constexpr int32_t POINTS_DIGITS = 3; // 500
        constexpr int32_t GLYPH         = 8;

        struct Crate final
        {
            bool broken{ false };
        };

        // a placeholder until there's a medal sprite. The collider sits on its own sprite-less entity,
        // on a digit it would assert, hud.nomask has no masks
        struct PointsPickup final
        {
            entity_id digits[POINTS_DIGITS];
        };


        void drop_weapon(Scene& scene, const ivec2 pos, const WeaponType type)
        {
            const entity_id e = spawn_standing(scene, pos, type == WeaponType::RifleFireball
                                                               ? anim::Other::RifleFireballPickup
                                                               : anim::Other::RifleSprayPickup);
            scene.get<Sprite>(e).layer = DROP_LAYER;
            scene.set<WeaponUpgrade>(e, type);

            Collider& col = scene.set<Collider>(e);

            col.gravity               = 0.0f;
            col.collide_with_tile_map = false;

            col.layer        |= Layer::WeaponUpgrade;
            col.target_layer |= Layer::Player;

            col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
            {
                // legs and body both hit it on the same tick
                Collider& c = scene.get<Collider>(self);
                if (!c.enabled) return;
                c.enabled = false;

                const Sounds& snd = MetalSlug::user_ctx().sounds;
                snd.play(Sfx::Pickup);
                snd.play(scene.get<WeaponUpgrade>(self).type == WeaponType::RifleFireball ? Sfx::FlameShot : Sfx::HeavyMachineGun);

                // destroyed and not just hidden like the old ones, there are a lot more of them now.
                // It's deferred so the player's callback still reads WeaponUpgrade fine this tick
                scene.destroy_entity(self);
            };
        }

        void drop_points(Scene& scene, const ivec2 pos)
        {
            entity_id digits[POINTS_DIGITS];
            uint32_t  value = Pickups::DROP_POINTS;

            for (int32_t i = POINTS_DIGITS - 1; i >= 0; --i, value /= 10)
            {
                const ivec2 center{ (2 * i + 1 - POINTS_DIGITS) * GLYPH / 2, -GLYPH / 2 };

                digits[i] = scene.create_entity();
                scene.set<Transform>(digits[i], pos + center);

                Sprite& s = scene.set<Sprite>(digits[i], anim::Hud::SmallFontYellow);
                s.force_frame(FONT_FIRST_DIGIT + value % 10);
                s.layer = DROP_LAYER;
            }

            const entity_id e = scene.create_entity();
            scene.set<Transform>(e, pos - ivec2{ 0, GLYPH / 2 });

            PointsPickup& p = scene.set<PointsPickup>(e);
            for (int32_t i = 0; i < POINTS_DIGITS; ++i) p.digits[i] = digits[i];

            Collider& col = scene.set<Collider>(e, ivec2{ 0 }, ivec2{ POINTS_DIGITS * GLYPH, GLYPH });

            col.gravity               = 0.0f;
            col.collide_with_tile_map = false;

            col.layer        |= Layer::PointsPickup;
            col.target_layer |= Layer::Player;

            col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
            {
                Collider& c = scene.get<Collider>(self);
                if (!c.enabled) return;
                c.enabled = false;

                MetalSlug::user_ctx().player.add_score(Pickups::DROP_POINTS);
                MetalSlug::user_ctx().sounds.play(Sfx::Pickup);

                for (const entity_id d : scene.get<PointsPickup>(self).digits) scene.destroy_entity(d);
                scene.destroy_entity(self);
            };
        }
    }


    entity_id Pickups::spawn_crate(Scene& scene, const ivec2 pos) const
    {
        const entity_id crate = spawn_standing(scene, pos, anim::Other::Crate);

        Sprite& s = scene.get<Sprite>(crate);
        s.play(anim::Other::Crate, CRATE_TICKS, is_reversed::no, should_loop::yes);
        s.layer = CRATE_LAYER;

        scene.set<Crate>(crate);

        // keeps the gravity, the spawn position is a tile above the floor
        Collider& col    = scene.set<Collider>(crate);
        col.layer        |= Layer::Crate;
        col.target_layer |= Layer::PlayerProjectile;

        col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
        {
            // a spray burst can put two bullets in it on one tick
            Collider& c = scene.get<Collider>(self);
            if (!c.enabled) return;
            c.enabled = false;

            scene.get<Crate>(self).broken = true;
        };

        return crate;
    }

    void Pickups::drop(Scene& scene, const ivec2 pos, const Drop what) const
    {
        switch (what)
        {
            case Drop::RifleSpray: drop_weapon(scene, pos, WeaponType::RifleSpray); break;
            case Drop::RifleFireball: drop_weapon(scene, pos, WeaponType::RifleFireball); break;
            case Drop::Points: drop_points(scene, pos); break;
            default: assert(!"not implemented");
        }
    }

    void Pickups::drop_random(Scene& scene, const ivec2 pos) const
    {
        drop(scene, pos, (Drop)App::ctx().rng.range<uint32_t>(0, (uint32_t)Drop::COUNT - 1));
    }

    void Pickups::update(Scene& scene) const
    {
        for (const entity_id crate : scene.pool<Crate>().owners())
        {
            if (!scene.get<Crate>(crate).broken) continue;
            scene.get<Crate>(crate).broken = false; // still in the pool until the flush

            const aabb  box = scene.get<Collider>(crate).bounds(scene.get<Transform>(crate));
            const ivec2 floor{ (box.min.x + box.max.x) / 2, box.max.y };

            scene.destroy_entity(crate);

            App::ctx().particles.emit(anim::Vfx::SmallExplosion, {
                .position  = standing_on(anim::Vfx::SmallExplosion, floor),
                .frame_dur = EXPLOSION_TICKS,
                .layer     = EXPLOSION_LAYER
            });

            MetalSlug::user_ctx().sounds.explosion();

            drop_random(scene, floor);
        }
    }
}
