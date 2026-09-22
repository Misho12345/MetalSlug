#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct PrivCtx;

    enum class TileType : uint8_t
    {
        Air,
        Floor,
        // Ladder,
        // OneWay
    };

    /**
     * @brief Loads and manages tile map (collider only)
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class TileMap final
    {
    public:
        TileMap(const TileMap&)            = delete;
        TileMap(TileMap&&)                 = delete;
        TileMap& operator=(const TileMap&) = delete;
        TileMap& operator=(TileMap&&)      = delete;

        [[nodiscard]]
        bool load(string_view path);

        [[nodiscard]] glm::ivec2 size() const { return size_; }
        [[nodiscard]] const TileType* data() const { return data_.data(); }

    private:
        TileMap() = default;

        vector<TileType> data_;
        glm::ivec2       size_{};

        friend ::mse::PrivCtx;
    };
}
