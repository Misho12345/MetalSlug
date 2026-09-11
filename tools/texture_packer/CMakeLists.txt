add_executable(MSTexturePacker)

file(GLOB_RECURSE TEXTURE_LOADER_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

find_package(nlohmann_json CONFIG REQUIRED)

target_sources(MSTexturePacker PRIVATE ${TEXTURE_LOADER_SOURCES})
target_link_libraries(MSTexturePacker PUBLIC STB MSCommon nlohmann_json::nlohmann_json)
target_include_directories(MSTexturePacker PRIVATE ${CMAKE_SOURCE_DIR}/engine/include/)

set(stamp_file "${texture_packer_out_dir}/stamp.txt")

function(is_leaf_dir dir out)
    file(GLOB children "${dir}/*")
    set(is_leaf TRUE)
    foreach (child IN LISTS children)
        if (IS_DIRECTORY ${child})
            set(is_leaf FALSE)
            break()
        endif ()
    endforeach ()
    set(${out} ${is_leaf} PARENT_SCOPE)
endfunction()

function(get_images dir out)
    file(GLOB images "${dir}/*.png" "${dir}/*.jpg")
    set(${out} ${images} PARENT_SCOPE)
endfunction()

function(snake_to_pascal_case str out)
    string(REPLACE "_" ";" word_list "${str}")

    set(result "")
    foreach (word IN LISTS word_list)
        if (word)
            string(SUBSTRING "${word}" 0 1 first_char)
            string(TOUPPER "${first_char}" first_char)
            string(LENGTH "${word}" word_len)
            if (word_len GREATER 1)
                string(SUBSTRING "${word}" 1 -1 rest)
                string(APPEND result "${first_char}${rest}")
            else ()
                string(APPEND result "${first_char}")
            endif ()
        endif ()
    endforeach ()

    set(${out} "${result}" PARENT_SCOPE)
endfunction()

set(args)
set(anim_header "#pragma once
#include <type_traits>

namespace mse::anim {
    template <typename E>
    inline constexpr int sprite_id{ -1 };
")

set(sprites_dir "${CMAKE_SOURCE_DIR}/assets/images/sprites")

file(GLOB_RECURSE all_items CONFIGURE_DEPENDS LIST_DIRECTORIES true "${sprites_dir}/*")

set(leaf_dirs)
foreach (item IN LISTS all_items)
    if (IS_DIRECTORY "${item}")
        is_leaf_dir("${item}" is_leaf)
        if (is_leaf)
            list(APPEND leaf_dirs "${item}")
        endif ()
    endif ()
endforeach ()

set(sprites_paths)

set(idx 0)
set(anim_count 0)
set(sprites_names)

foreach (dir IN LISTS leaf_dirs)
    get_images("${dir}" images)
    if (NOT images)
        continue()
    endif ()

    list(APPEND sprites_paths ${images})
    list(LENGTH images num_images)
    list(APPEND args ${idx} ${num_images} ${images})

    cmake_path(GET dir FILENAME leaf_name)
    snake_to_pascal_case("${leaf_name}" enum_name)

    cmake_path(GET dir PARENT_PATH parent_dir)
    cmake_path(RELATIVE_PATH parent_dir BASE_DIRECTORY "${sprites_dir}" OUTPUT_VARIABLE rel_parent)

    set(ns_prefix "")
    set(full_enum_type "${enum_name}")

    if (NOT rel_parent STREQUAL "" AND NOT rel_parent STREQUAL ".")
        string(REPLACE "/" "::" ns_prefix "${rel_parent}")
        set(full_enum_type "${ns_prefix}::${enum_name}")
    endif ()

    list(APPEND sprites_names "${full_enum_type}")

    set(enum_values "")
    foreach (img IN LISTS images)
        cmake_path(GET img STEM img_stem)
        snake_to_pascal_case("${img_stem}" img_pascal)
        string(APPEND enum_values "        ${img_pascal},\n")

        math(EXPR anim_count "${anim_count} + 1")
    endforeach ()

    if (NOT ns_prefix STREQUAL "")
        string(APPEND anim_header "
    namespace ${ns_prefix}
    {
        enum class ${enum_name}
        {
${enum_values}            _COUNT
        };
    }

    template <> inline constexpr int sprite_id<${full_enum_type}> = ${idx};\n")

    else ()
        string(APPEND anim_header "
    enum class ${enum_name}
    {
${enum_values}        _COUNT
    };

    template <> inline constexpr int sprite_id<${full_enum_type}> = ${idx};\n")
    endif ()

    math(EXPR idx "${idx} + 1")
endforeach ()

list(APPEND args "--out" ${texture_packer_out_dir})

set(sprite_anims_count_cases "")
foreach (name IN LISTS sprites_names)
    string(APPEND sprite_anims_count_cases "
        case sprite_id<${name}>: return static_cast<uint32_t>(${name}::_COUNT);")
endforeach ()

string(APPEND anim_header "
    template <typename E>
    concept sprite_enum = std::is_enum_v<E> && requires
    {
        { anim::sprite_id<E> } -> std::convertible_to<int>;
    } && anim::sprite_id<E> >= 0;

    struct Info { int sprite_id, anim_id; };

    template <sprite_enum E>
    constexpr Info info(E e) { return { sprite_id<E>, static_cast<int>(e) }; }

    constexpr inline int sprite_count = ${idx};
    constexpr inline int total_anim_count = ${anim_count};

    inline uint32_t sprite_anim_count(const int _sprite_id) noexcept
    {
        switch (_sprite_id)
        {${sprite_anims_count_cases}
        }
    }
}")


add_custom_command(
        OUTPUT ${stamp_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${texture_packer_out_dir}
        COMMAND $<TARGET_FILE:MSTexturePacker> ${args}
        COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file}
        DEPENDS MSTexturePacker ${sprites_paths}
        COMMENT "Packing textures into ${texture_packer_out_dir}"
        VERBATIM
)

file(GENERATE
        OUTPUT "${anim_header_path}"
        CONTENT "${anim_header}"
)

add_custom_target(TexturePacker ALL DEPENDS ${stamp_file})