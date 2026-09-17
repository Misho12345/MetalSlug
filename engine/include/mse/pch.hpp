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
        constexpr float operator""_deg(const long double deg) { return glm::radians(static_cast<float>(deg)); }
    }

    // for static_assert, because pre C++23 template independent expressions
    // that evaluate to false are not allowed
    template <typename T>
    inline constexpr bool always_false = false;

    // might replace later with a 16.16 integer range
    using unit = float;
}

#include "neo_geo_spec.hpp"

namespace mse
{
    using Target = NeoGeoSpec;
}