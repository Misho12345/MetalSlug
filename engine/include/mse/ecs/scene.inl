#pragma once

namespace mse
{
    template <typename C, typename... Args>
    C& Scene::set(entity_id entity, Args&&... args)
    {
        assert(valid(entity) && "Entity is not valid");

        // inject scene and entity id if the constructor of the component enables it
        if constexpr (requires { C(*this, entity, std::forward<Args>(args)...); })
        {
            return pool<C>().set(entity, *this, entity, std::forward<Args>(args)...);
        }
        else return pool<C>().set(entity, std::forward<Args>(args)...);
    }


    template <typename C>
    C* Scene::try_get(entity_id entity)
    {
        if (!valid(entity)) return nullptr;
        return pool<C>().get(entity);
    }

    template <typename C>
    const C* Scene::try_get(entity_id entity) const
    {
        if (!valid(entity)) return nullptr;
        return pool<C>().get(entity);
    }


    template <typename C>
    C& Scene::get(entity_id entity)
    {
        assert(valid(entity) && "Entity is not valid");
        assert(has<C>(entity) && "Entity does not have the requested component");
        return *pool<C>().get(entity);
    }

    template <typename C>
    const C& Scene::get(entity_id entity) const
    {
        assert(valid(entity) && "Entity is not valid");
        assert(has<C>(entity) && "Entity does not have the requested component");
        return *pool<C>().get(entity);
    }


    template <typename C>
    bool Scene::has(entity_id entity) const
    {
        assert(valid(entity) && "Entity is not valid");
        return pool<C>().has(entity);
    }


    template <typename C>
    void Scene::remove(entity_id entity)
    {
        assert(valid(entity) && "Entity is not valid");
        pool<C>().remove(entity);
    }


    inline bool Scene::valid(const entity_id entity) const
    {
        return entity &&
                entity.idx() < versions_.size() &&
                versions_[entity.idx()] == entity.version();
    }


    template <typename C>
    ComponentPool<C>& Scene::pool()
    {
        static const size_t idx = component_type_index<C>();

        while (pools_.size() <= idx) pools_.emplace_back();

        if (!pools_[idx].get()) pools_[idx] = new ComponentPool<C>();
        return (ComponentPool<C>&)*pools_[idx];
    }

    template <typename C>
    const ComponentPool<C>& Scene::pool() const
    {
        static const size_t idx = component_type_index<C>();

        if (idx < pools_.size() && pools_[idx].get()) return (const ComponentPool<C>&)*pools_[idx];

        #ifndef NDEBUG
        printf("WARN: requesting access to a non-existing pool "
               "(called through has/get/try_get of never used type before), "
               "empty pool is returned so the code will work, "
               "but relying on that is not recommended.");
        #endif

        static const ComponentPool<C> empty;
        return empty;
    }
}
