#pragma once

#include "api.hpp"

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>

#include "common/common.hpp"
#include "anim.hpp"

#include <new>
#include <concepts>


#include <nlohmann/json.hpp>
using nlohmann::json;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext.hpp>

inline float operator""_deg(const long double deg) { return glm::radians(static_cast<float>(deg)); }
