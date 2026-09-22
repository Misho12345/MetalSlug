#include "mse/pch.hpp"
#include "mse/ecs/scene.hpp"

namespace mse
{
    Scene::Scene() : camera_{ create_entity() }
    {
        free_entities_.reserve(100);
        versions_.reserve(100);

        set<Transform>(camera_);
    }

    entity_id Scene::create_entity()
    {
        uint32_t idx;

        if (free_entities_.empty())
        {
            idx = next_++;
            versions_.emplace_back(0);
        }
        else
        {
            idx = free_entities_.back();
            free_entities_.pop_back();
        }

        return { idx, versions_[idx] };
    }

    void Scene::destroy_entity(const entity_id entity)
    {
        if (!valid(entity)) return;

        ++versions_[entity.idx()];
        free_entities_.emplace_back(entity.idx());

        transform_pool_.remove(entity);
        sprite_renderer_pool_.remove(entity);
        sprite_renderer_pool_.remove(entity);
    }
}
