#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct GameCtx;

    enum class TileType : uint8_t
    {
        Air,
        Floor,
        Platform,
        SlopeL,
        SlopeR
    };

    /**
     * @brief Loads and manages tile map (collider only)
     * @note Owned by GameCtx, not constructable by anything else and not movable or copyable
     */
    class MSE_API TileMap final
    {
    public:
        TileMap(const TileMap&)            = delete;
        TileMap(TileMap&&)                 = delete;
        TileMap& operator=(const TileMap&) = delete;
        TileMap& operator=(TileMap&&)      = delete;

        [[nodiscard]]
        bool load(string_view path);

        [[nodiscard]] TileType* data() { return data_.data(); }
        [[nodiscard]] const TileType* data() const { return data_.data(); }

        [[nodiscard]]
        uvec2 size() const { return size_; }

        TileType& operator[](const uvec2 coords) { return data_[coords.x + coords.y * size_.x]; }
        const TileType& operator[](const uvec2 coords) const { return data_[coords.x + coords.y * size_.x]; }

    private:
        TileMap() = default;

        vector<TileType> data_{ nullptr };
        uvec2 size_{};

        friend ::mse::GameCtx;
    };
}
