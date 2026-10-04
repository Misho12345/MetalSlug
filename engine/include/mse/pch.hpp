#pragma once

#include "api.hpp"

// C headers
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>

// C++ headers
#include <concepts>
#include <type_traits>
#include <new>

// Engine-related headers
#include "common/common.hpp"
    #include "anim.hpp"

// Third-party libraries
#include <nlohmann/json.hpp>
using nlohmann::json;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifndef NDEBUG
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

#include <stb_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext.hpp>


namespace mse
{
    // for static_assert, because pre C++23 template independent expressions
    // that evaluate to false are technically not allowed
    template <typename T>
    inline constexpr bool always_false = false;

    // might replace later with a 16.16 integer range
    using unit = float;



    using mat4 = glm::f32mat4;

    using vec2 = glm::f32vec2;
    using ivec2 = glm::i32vec2;
    using uvec2 = glm::u32vec2;

    using color = glm::u8vec4;


    namespace literals
    {
        /**
         * @brief Converts degrees to radians
         * @param deg Number literal in degrees
         * @return Angle in radians
         *
         * Example:
         * @code float angle = 90.0_deg; @endcode
         */
        constexpr float operator""_deg(const long double deg) { return glm::radians((float)(deg)); }


        /**
         * @brief Constructs a color object from an integer
         * @param val Color value
         * @return glm::u8vec4 with the color value (alpha = 255)
         *
         * Example:
         * @code color c = 0xff00aa_rgb; @endcode
         */
        constexpr color operator""_rgb(const unsigned long long val)
        {
            return {
                (uint8_t)(val >> 16 & 0xFF),
                (uint8_t)(val >> 8 & 0xFF),
                (uint8_t)(val & 0xFF),
                255
            };
        }

        /**
         * @brief Constructs a color object from an integer
         * @param val Color value
         * @return glm::u8vec4 with the color value
         *
         * Example:
         * @code color c = 0xff00aaff_rgba; @endcode
         */
        constexpr color operator""_rgba(const unsigned long long val)
        {
            return {
                (uint8_t)(val >> 24 & 0xFF),
                (uint8_t)(val >> 16 & 0xFF),
                (uint8_t)(val >> 8 & 0xFF),
                (uint8_t)(val & 0xFF)
            };
        }
    }
}

#include "neo_geo_spec.hpp"

namespace mse
{
    using Target = NeoGeoSpec;

    static_assert(
        std::same_as<std::remove_cvref_t<decltype(Target::RESOLUTION)>, uvec2> &&
        std::same_as<std::remove_cvref_t<decltype(Target::FRAME_TIME)>, float> &&
        std::same_as<std::remove_cvref_t<decltype(Target::PAR)>, float> &&
        std::same_as<std::remove_cvref_t<decltype(Target::TILE_SIZE)>, uvec2> &&
        std::same_as<std::remove_cvref_t<decltype(Target::DAR)>, float>,
        "invalid Target format");

    static_assert(
        Target::RESOLUTION.x % Target::TILE_SIZE.x == 0 &&
        Target::RESOLUTION.y % Target::TILE_SIZE.y == 0,
        "Target::RESOLUTION must be divisible by Target::TILE_SIZE");

    inline const uvec2 HALF_RESOLUTION = Target::RESOLUTION / 2;

    inline constexpr ivec2 RESOLUTION_i = Target::RESOLUTION;
    inline const ivec2 HALF_RESOLUTION_i = HALF_RESOLUTION;

    inline constexpr ivec2 TILE_SIZE_i = Target::TILE_SIZE;
}