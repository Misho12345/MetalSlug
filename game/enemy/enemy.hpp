#pragma once
#include "mse/mse.hpp"

namespace ms
{
    // on every enemy, next to its kind's own data
    struct Enemy final
    {
        int8_t facing{ -1 }; // all the enemy art faces left
        int8_t shot_dir{};   // which way the bullet that killed it was going, 0 for straight up or down

        uint16_t corpse_ticks{};

        bool shot{ false };
        bool burning{ false };
    };

    struct EnemyAnimData final
    {
        uint8_t    ticks;
        mse::ivec2 origin; // sprite center from the bottom middle of the box
        bool       loop{ false };
    };


    class EnemyManager
    {
    public:
        virtual ~EnemyManager() = default;

        virtual void register_machine() const = 0;

        // pos is what it stands on (or floats at), the bottom middle of its box
        virtual mse::entity_id spawn(mse::Scene& scene, mse::ivec2 pos) const = 0;
    };


    // bottom middle of the collider
    mse::ivec2 feet(const mse::Scene& scene, mse::entity_id e);

    // player's feet minus e's
    mse::ivec2 to_player(const mse::Scene& scene, mse::entity_id e);

    // puts the sprite on the box, the box stays where it is, it's what stands on the floor
    void anchor(mse::Scene& scene, mse::entity_id e, mse::ivec2 origin);

    template <mse::anim::sprite_enum E>
    void play_anchored(mse::Scene& scene, const mse::entity_id e, const E a, const EnemyAnimData& d)
    {
        scene.get<mse::Sprite>(e).play(a, d.ticks, mse::is_reversed::no, d.loop, mse::restart_if_same::yes);
        anchor(scene, e, d.origin);
    }

    // the callback for a collider targeting PlayerProjectile
    void on_enemy_shot(mse::Scene& scene, mse::entity_id self, mse::entity_id other, uint32_t);

    // not something anything collides with any more, but it still falls
    void stop_colliding(mse::Scene& scene, mse::entity_id e);

    // call every tick after the death animation, destroys it once it's blinked out
    void blink_out(mse::Scene& scene, mse::entity_id e);
}
