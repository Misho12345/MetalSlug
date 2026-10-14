# this is a shared library that contains common code for the project
# that includes utility functions, containers and other common code that can be used by other modules

add_library(MSECommon SHARED)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

target_sources(MSECommon PRIVATE ${COMMON_SOURCES})
target_include_directories(MSECommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(MSECommon PRIVATE MS_COMMON_EXPORTS)

enable_warnings(MSECommon)