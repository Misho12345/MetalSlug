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
    using glm::bvec2;

    struct color_t final
    {
        constexpr color_t() { data_u32 = ~0u; }

        constexpr color_t(const uint8_t r_, const uint8_t g_, const uint8_t b_, const uint8_t a_)
        {
            r = r_;
            g = g_;
            b = b_;
            a = a_;
        }

        constexpr color_t(const glm::u8vec4 data_) { data = data_; }

        constexpr explicit color_t(const uint32_t data_) { data_u32 = data_; }

        union
        {
            struct { uint8_t r, g, b, a; };
            glm::u8vec4 data;
            uint32_t data_u32;
        };
    };

    template <typename>
    struct bool_value final
    {
        enum value : bool { no, yes };

        constexpr bool_value(bool&&) = delete;
        constexpr bool_value(const bool& v) : v_{ v } {}
        constexpr bool_value(const value v) : v_{ v } {}
        constexpr operator bool() const { return v_; }

        constexpr bool_value& operator=(const value v)
        {
            v_ = v;
            return *this;
        }

    private:
        value v_{};
    };

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
        constexpr color_t operator""_rgb(const unsigned long long val)
        {
            return color_t{ (uint32_t)val << 8 | 0xff };
        }

        /**
         * @brief Constructs a color object from an integer
         * @param val Color value
         * @return glm::u8vec4 with the color value
         *
         * Example:
         * @code color c = 0xff00aaff_rgba; @endcode
         */
        constexpr color_t operator""_rgba(const unsigned long long val)
        {
            return color_t{ (uint32_t)val };
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