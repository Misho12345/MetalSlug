#pragma once

// NeoGeo emulation settings
struct NeoGeoSpec final
{
    glm::ivec2 resolution{ 304, 224 };
    unit       frame_time{ 0.016896 };      // 264 / 15625
    unit       pixel_aspect_ratio{ 1.0 };
    glm::ivec2 tile_size{ 16, 16 };
};
