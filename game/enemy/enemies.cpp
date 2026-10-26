#include "enemies.hpp"
#include "grenade.hpp"
#include "missile.hpp"

namespace ms
{
    using namespace mse;

    void Enemies::register_machines() const
    {
        for (uint8_t k = 0; k < (uint8_t)EnemyKind::COUNT; ++k) type((EnemyKind)k).register_machine();

        register_grenade_machine();
        register_missile_machine();
    }

    entity_id Enemies::spawn(Scene& scene, const ivec2 pos, const EnemyKind kind) const
    {
        return type(kind).spawn(scene, pos);
    }

    const EnemyManager& Enemies::type(const EnemyKind kind) const
    {
        switch (kind)
        {
            case EnemyKind::Rebel: return rebel_;
            case EnemyKind::AquaRebel: return aqua_rebel_;
            default: assert(!"not implemented");
        }

        // just something for the compiler to not complain
        return rebel_;
    }
}
