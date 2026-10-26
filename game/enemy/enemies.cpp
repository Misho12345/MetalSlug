#include "enemies.hpp"
#include "grenade.hpp"
#include "missile.hpp"

namespace ms
{
    using namespace mse;

    void Enemies::register_machines() const
    {
        for (uint8_t k = 0; k < (uint8_t)EnemyKind::COUNT; ++k) type((EnemyKind)k).register_machine();

        // not by the managers, two kinds throwing grenades would add the machine twice and tick them double
        register_grenade_machine();
        register_missile_machine();
    }

    entity_id Enemies::spawn(Scene& scene, const ivec2 pos, const EnemyKind kind) const
    {
        return type(kind).spawn(scene, pos);
    }

    const EnemyManager& Enemies::type(const EnemyKind kind) const
    {
        // no default, so clang/gcc flag a kind without a case at compile time instead of it reaching the assert
        switch (kind)
        {
            case EnemyKind::Rebel: return rebel_;
            case EnemyKind::AquaRebel: return aqua_rebel_;
            case EnemyKind::COUNT: break;
        }

        assert(!"not implemented");
        return rebel_;
    }
}
