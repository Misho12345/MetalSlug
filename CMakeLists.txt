cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(MetalSlug LANGUAGES CXX)


option(MSE_USE_MIMALLOC "Use mimalloc" ON)
option(MSE_ENABLE_ASAN "Address sanitizer" OFF)
option(MSE_ENABLE_UBSAN "UB sanitizer" OFF)

set(anim_header_path "${CMAKE_BINARY_DIR}/include/anim.hpp")
set(texture_packer_out_dir "${CMAKE_BINARY_DIR}/texture_packer")


list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CompilerFlags)
include(LinkMimalloc)
include(MseUtil)
include(MseExecutable)

add_subdirectory(third_party/stb)
add_subdirectory(third_party/miniaudio)

add_subdirectory(common)
add_subdirectory(tools/texture_packer)
add_subdirectory(engine)
add_subdirectory(game)

