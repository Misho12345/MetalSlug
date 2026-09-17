# this is a shared library that contains common code for the project
# that includes utility functions, containers and other common code that can be used by other modules

add_library(MSCommon SHARED)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

target_sources(MSCommon PRIVATE ${COMMON_SOURCES})
target_include_directories(MSCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(MSCommon PRIVATE MS_COMMON_EXPORTS)