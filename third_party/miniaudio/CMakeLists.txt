set(ma_impl_file "${CMAKE_CURRENT_BINARY_DIR}/ma_impl.cpp")

find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")

file(WRITE "${ma_impl_file}" [[
    #define MINIAUDIO_IMPLEMENTATION
    #include "miniaudio.h"
]])

add_library(Miniaudio OBJECT ${ma_impl_file})
set_target_properties(Miniaudio PROPERTIES POSITION_INDEPENDENT_CODE ON) # because it will be linked into other shared libraries
target_include_directories(Miniaudio PUBLIC ${MINIAUDIO_INCLUDE_DIRS})