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
        C& set(entity_id entity, Args&&... args);

        template <typename C>
        [[nodiscard]]
        C* try_get(entity_id entity);

        template <typename C>
        [[nodiscard]]
        const C* try_get(entity_id entity) const;

        template <typename C>
        [[nodiscard]]
        C& get(entity_id entity);

        template <typename C>
        [[nodiscard]]
        const C& get(entity_id entity) const;

        template <typename C>
        [[nodiscard]]
        bool has(entity_id entity) const;

        template <typename C>
        void remove(entity_id entity);

        [[nodiscard]]
        bool valid(const entity_id entity) const;

        [[nodiscard]]
        entity_id camera() const { return camera_; }


        template <typename C>
        [[nodiscard]]
        ComponentPool<C>& pool();

        template <typename C>
        [[nodiscard]]
        const ComponentPool<C>& pool() const;


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

#include "scene.inl"
