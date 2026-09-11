add_library(MSCommon SHARED
        include/common/algorithm.hpp
        include/common/common.hpp)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

target_sources(MSCommon PRIVATE ${COMMON_SOURCES})
target_include_directories(MSCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(MSCommon PRIVATE MS_COMMON_EXPORTS)