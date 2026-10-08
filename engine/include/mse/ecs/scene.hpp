#pragma once
#include "mse/api.hpp"
#include "components.hpp"
#include "component_pool.hpp"
#include "component_type.hpp"

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

        [[nodiscard]]
        entity_id create_entity();
        void      destroy_entity(entity_id entity);

        template <typename C, typename... Args>
        C& set(entity_id entity, Args&&... args)
        {
            assert(valid(entity) && "Entity is not valid");

            // inject scene and entity id if the constructor of the component enables it
            if constexpr (requires{ C(*this, entity, std::forward<Args>(args)...); })
            {
                return pool<C>().set(entity, *this, entity, std::forward<Args>(args)...);
            }
            else return pool<C>().set(entity, std::forward<Args>(args)...);
        }

        template <typename C>
        [[nodiscard]]
        C* try_get(entity_id entity)
        {
            if (!valid(entity)) return nullptr;
            return pool<C>().get(entity);
        }

        template <typename C>
        [[nodiscard]]
        const C* try_get(entity_id entity) const
        {
            if (!valid(entity)) return nullptr;
            return pool<C>().get(entity);
        }

        template <typename C>
        [[nodiscard]]
        C& get(entity_id entity)
        {
            assert(valid(entity) && "Entity is not valid");
            assert(has<C>(entity) && "Entity does not have the requested component");
            return *pool<C>().get(entity);
        }

        template <typename C>
        [[nodiscard]]
        const C& get(entity_id entity) const
        {
            assert(valid(entity) && "Entity is not valid");
            assert(has<C>(entity) && "Entity does not have the requested component");
            return *pool<C>().get(entity);
        }

        template <typename C>
        [[nodiscard]]
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

        [[nodiscard]]
        bool valid(const entity_id entity) const
        {
            return entity &&
                    entity.idx() < versions_.size() &&
                    versions_[entity.idx()] == entity.version();
        }

        [[nodiscard]]
        entity_id camera() const { return camera_; }


        template <typename C>
        [[nodiscard]]
        ComponentPool<C>& pool()
        {
            static const size_t idx = component_type_index<C>();

            while (pools_.size() <= idx) pools_.emplace_back();

            if (!pools_[idx].get()) pools_[idx] = new ComponentPool<C>();
            return (ComponentPool<C>&)*pools_[idx];
        }

        template <typename C>
        [[nodiscard]]
        const ComponentPool<C>& pool() const
        {
            static const size_t idx = component_type_index<C>();

            if (idx < pools_.size() && pools_[idx].get()) return (const ComponentPool<C>&)*pools_[idx];

            static const ComponentPool<C> empty;
            return empty;
        }


        [[nodiscard]] ivec2 camera_pos() const { return get<Transform>(camera_).position; }
        [[nodiscard]] ivec2 camera_pos_screen() const { return camera_pos() - HALF_RESOLUTION_i; }

        entity_id bg_entity{};

    private:
        vector<unique_ptr<IComponentPool>> pools_;

        vector<uint8_t>  versions_{};
        vector<uint32_t> free_entities_{};

        uint32_t  next_{};
        entity_id camera_;
    };
}
