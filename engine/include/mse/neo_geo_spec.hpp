#pragma once

namespace mse
{
    // NeoGeo emulation settings
    struct NeoGeoSpec final
    {
        // MetalSlug doesn't use 8 pixels to the left and right of the screen
        static constexpr glm::ivec2 RESOLUTION{ 304, 224 };

        // 264 / 15625
        static constexpr float FRAME_TIME{ 0.016896f };

        // pixel aspect ratio
        static constexpr float PAR{ 14.0f / 15.0f };

        static constexpr glm::ivec2 TILE_SIZE{ 16, 16 };
    };
}
