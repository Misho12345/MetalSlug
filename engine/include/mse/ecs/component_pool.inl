#pragma once

namespace mse
{
    template <typename C>
    template <typename... Args> requires std::constructible_from<C, Args...>
    C& ComponentPool<C>::set(const entity_id id, Args&&... args)
    {
        if (id.idx() >= lookup_.size()) lookup_.resize(id.idx() + 1, -1);

        if (lookup_[id.idx()] != -1)
        {
            return components_[(size_t)lookup_[id.idx()]] = C{ std::forward<Args>(args)... };
        }

        lookup_[(size_t)id.idx()] = (int32_t)components_.size();
        owners_.emplace_back(id);
        components_.emplace_back(std::forward<Args>(args)...);
        return components_.back();
    }

    template <typename C>
    const C* ComponentPool<C>::get(const entity_id id) const
    {
        if (id.idx() >= lookup_.size() || lookup_[id.idx()] == -1) return nullptr;
        return &components_[(size_t)lookup_[id.idx()]];
    }

    template <typename C>
    bool ComponentPool<C>::has(const entity_id id) const
    {
        return id.idx() < lookup_.size() && lookup_[id.idx()] != -1;
    }

    template <typename C>
    void ComponentPool<C>::remove(const entity_id id)
    {
        if (!has(id)) return;

        size_t idx = (size_t)lookup_[id.idx()];
        size_t last_idx = components_.size() - 1_zu;
        const entity_id last_entity = owners_[last_idx];

        components_[idx] = std::move(components_[last_idx]);
        owners_[idx] = last_entity;
        lookup_[last_entity.idx()] = (int32_t)idx;

        components_.pop_back();
        owners_.pop_back();
        lookup_[id.idx()] = -1;
    }
}
