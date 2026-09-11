find_package(Stb REQUIRED)

set(stbi_impl_file "${CMAKE_CURRENT_BINARY_DIR}/stbi_impl.cpp")

file(WRITE "${stbi_impl_file}" [[
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include <stb_image_write.h>
]])

add_library(STB OBJECT ${stbi_impl_file})
set_target_properties(STB PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(STB PUBLIC ${Stb_INCLUDE_DIR})