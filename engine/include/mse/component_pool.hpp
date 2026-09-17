#pragma once
#include "pch.hpp"
#include "entity_id.hpp"

namespace mse
{
    /**
     * @brief A pool of components of type C
     * @details Stores components in a packed vector and maintains a lookup table for fast access by entity_id
     * @tparam C The type of component stored in the pool
     * @note This is only for use by the Scene class
     * @see scene.hpp
     */
    template <typename C>
    class ComponentPool final
    {
    public:
        ComponentPool() = default;

        /**
         * @brief Set/Create the component for the given entity_id
         * @tparam Args The types of the arguments to pass to the constructor of C
         * @param id The entity_id to set the component for
         * @param args The arguments to pass to the constructor of C
         * @return A reference to the component
         */
        template <typename... Args> requires std::constructible_from<C, Args...>
        C& set(entity_id id, Args&&... args);

        /**
         * @brief Get the component for the given entity_id
         * @param id The entity_id to get the component for
         * @return A pointer to the component, or nullptr if the entity_id does not have a component
         */
        C* get(entity_id id) { return const_cast<C*>(std::as_const(*this).get(id)); }

        /**
         * @brief Get the component for the given entity_id
         * @param id The entity_id to get the component for
         * @return A pointer to the component, or nullptr if the entity_id does not have a component
         */
        const C* get(entity_id id) const;

        /**
         * @brief Check if the given entity_id has a component
         * @param id The entity_id to check
         * @return true if the entity_id has a component, false otherwise
         */
        bool has(entity_id id) const;

        /**
         * @brief Remove the component for the given entity_id
         * @param id The entity_id to remove the component for
         */
        void remove(entity_id id);

        vector<C>& components() { return components_; }
        const vector<C>& components() const { return components_; }

        const vector<entity_id>& owners() const { return owners_; }
        const vector<int32_t>& lookup() const { return lookup_; }


        entity_id get_id(C& c) const
        {
            if (c < components_.data() ||
                c >= components_.data() + components_.size())
                return {};

            return owners_[(
                reinterpret_cast<uint8_t*>(&c) -
                reinterpret_cast<uint8_t*>(components_.data())
            ) / sizeof(C)];
        }

    private:
        vector<C>         components_; // packed data
        vector<entity_id> owners_;     // idx of components_ -> entity_id
        vector<int32_t>   lookup_;     // entity_id -> idx of components_
    };
}

#include "component_pool.inl"
