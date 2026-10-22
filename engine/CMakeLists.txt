add_library(MetalSlugEngine SHARED)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

target_sources(MetalSlugEngine PRIVATE ${ENGINE_SOURCES})

find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)


target_link_libraries(MetalSlugEngine
        PUBLIC
        MSECommon
        glm::glm-header-only
        PRIVATE
        OpenGL::GL
        glad::glad
        imgui::imgui
        glfw
        nlohmann_json::nlohmann_json
        STB
        Miniaudio
)

add_dependencies(MetalSlugEngine TexturePacker)

target_include_directories(MetalSlugEngine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(MetalSlugEngine PRIVATE MSE_EXPORTS)
target_precompile_headers(MetalSlugEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mse/pch.hpp)

mse_apply_flags(MetalSlugEngine)
