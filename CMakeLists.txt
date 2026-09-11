cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(MetalSlug LANGUAGES CXX)

set(anim_header_path "${CMAKE_BINARY_DIR}/include/anim.hpp")
set(texture_packer_out_dir "${CMAKE_BINARY_DIR}/texture_packer")

add_subdirectory(third_party/stb)
add_subdirectory(common)
add_subdirectory(tools/texture_packer)
add_subdirectory(engine)
add_subdirectory(game)

