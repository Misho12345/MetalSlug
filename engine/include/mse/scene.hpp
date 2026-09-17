#pragma once
#include "api.hpp"
#include "components.hpp"
#include "component_pool.hpp"

namespace mse
{
    /**
     * @brief The scene class
     * @details The scene class manages the component pools and the camera object.
     */
    class MSE_API Scene final
    {
    public:
        Scene();

        entity_id create_entity();
        void      destroy_entity(entity_id entity);

        template <typename C, typename... Args>
        C& set(entity_id entity, Args&&... args)
        {
            assert(valid(entity) && "Entity is not valid");
            return pool<C>().set(entity, std::forward<Args>(args)...);
        }

        template <typename C>
        C* try_get(entity_id entity)
        {
            if (!valid(entity)) return nullptr;
            return pool<C>().get(entity);
        }

        template <typename C>
        const C* try_get(entity_id entity) const
        {
            if (!valid(entity)) return nullptr;
            return pool<C>().get(entity);
        }

        template <typename C>
        C& get(entity_id entity)
        {
            assert(valid(entity) && "Entity is not valid");
            assert(has<C>(entity) && "Entity does not have the requested component");
            return *pool<C>().get(entity);
        }

        template <typename C>
        const C& get(entity_id entity) const
        {
            assert(valid(entity) && "Entity is not valid");
            assert(has<C>(entity) && "Entity does not have the requested component");
            return *pool<C>().get(entity);
        }

        template <typename C>
        bool has(entity_id entity) const
        {
            assert(valid(entity) && "Entity is not valid");
            return pool<C>().has(entity);
        }

        template <typename C>
        void remove(entity_id entity)
        {
            assert(valid(entity) && "Entity is not valid");
            pool<C>().remove(entity);
        }

        bool valid(const entity_id entity) const
        {
            return entity &&
                    entity.idx() < versions_.size() &&
                    versions_[entity.idx()] == entity.version();
        }

        entity_id camera() const { return camera_; }


        template <typename C>
        ComponentPool<C>& pool()
        {
            return const_cast<ComponentPool<C>&>(std::as_const(*this).pool<C>());
        }

        template <typename C>
        const ComponentPool<C>& pool() const
        {
            if constexpr (std::same_as<C, Transform>) return transform_pool_;
            else if constexpr (std::same_as<C, Rigidbody>) return rigidbody_pool_;
            else if constexpr (std::same_as<C, BoxCollider>) return collider_pool_;
            else if constexpr (std::same_as<C, SpriteRenderer>) return sprite_renderer_pool_;
            else static_assert(always_false<C>, "Component type not supported in Scene");
        }

    private:
        ComponentPool<Transform>      transform_pool_;
        ComponentPool<Rigidbody>      rigidbody_pool_;
        ComponentPool<BoxCollider>    collider_pool_;
        ComponentPool<SpriteRenderer> sprite_renderer_pool_;

        vector<uint8_t>  versions_{};
        vector<uint32_t> free_entities_{};

        uint32_t  next_{};
        entity_id camera_;
    };
}
