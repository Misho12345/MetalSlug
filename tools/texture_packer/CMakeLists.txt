# the texture packer assembles all the sprite images into texture atlases
# it also generates a header file with the sprite enums and their corresponding ids for use in the game code

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


# stamp file to prevent unnecessary rebuilds of the texture packer output
set(stamp_file "${texture_packer_out_dir}/stamp.txt")

# checks if a directory contains only files and no subdirectories
# only such directories are considered an animation pack for 1 sprite
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
set(anim_header "// Automatically generated enums for sprite animations

#pragma once
#include <type_traits>
#include <concepts>
#include <cstdint>

namespace mse::anim {
    // sprite_id is a template variable that maps an enum type to its corresponding sprite id
    // if the enum type is not a valid sprite enum, sprite_id will be -1
    template <typename E>
    inline constexpr int sprite_id{ -1 };
")

set(sprites_dir "${CMAKE_SOURCE_DIR}/assets/images/sprites")

# Gather all directories that contain sprite images (leaf directories)
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

set(idx 0) # sprite id counter
set(anim_count 0) # total animation count
set(sprites_names) # list of sprite enum names

# Process each leaf directory to gather images and generate enum names
foreach (dir IN LISTS leaf_dirs)
    get_images("${dir}" images)
    if (NOT images)
        continue()
    endif ()

    list(APPEND sprites_paths ${images})
    list(LENGTH images num_images)
    list(APPEND args ${idx} ${num_images})

    foreach (img IN LISTS images)
        list(APPEND args ${img})
        cmake_path(GET img STEM LAST_ONLY frame_count)      # idle.4.png -> idle.4
        cmake_path(GET frame_count EXTENSION frame_count)   # idle.4 -> .4
        string(REPLACE "." "" frame_count "${frame_count}") # .4 -> 4
        list(APPEND args ${frame_count})
    endforeach ()

    # get the needed info to construct the enum name and namespace
    # for example this:
    # assets/images/sprites
    #├── enemy
    #│   ├── idle.4.jpg
    #│   ├── walking.4.jpg
    #│   └── shooting.7.png
    #└── player
    #    ├── body
    #    │   ├── idle.4.png
    #    │   ├── drinking.15.png
    #    │   └── falling.4.png
    #    └── legs
    #        ├── idle.4.png
    #        ├── jumping.4.png
    #        └── walking.4.png
    # the enum will be:
    # enum class Enemy { Idle, Walking, Shooting, _COUNT };
    # namespace player
    # {
    #     enum class Body { Idle, Drinking, Falling, _COUNT };
    # }
    # namespace player
    # {
    #     enum class Legs { Idle, Jumping, Walking, _COUNT };
    # }

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

    # generate the enum values for the images in this directory
    set(enum_values "")
    foreach (img IN LISTS images)
        cmake_path(GET img STEM img_stem)

        snake_to_pascal_case("${img_stem}" img_pascal)
        string(APPEND enum_values "        ${img_pascal},\n")

        math(EXPR anim_count "${anim_count} + 1")
    endforeach ()

    # append the correct enum definition to the header file content
    # depending on whether there is a need for a namespace
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

# Generate the switch cases for sprite_anim_count function
set(sprite_anims_count_cases "")
foreach (name IN LISTS sprites_names)
    string(APPEND sprite_anims_count_cases "
        case sprite_id<${name}>: return static_cast<uint32_t>(${name}::_COUNT);")
endforeach ()

string(APPEND anim_header "
    // sprite_enum concept ensures that the type is an enum and
    // has a valid sprite_id and not just a random enum type
    template <typename E>
    concept sprite_enum = std::is_enum_v<E> && requires
    {
        { anim::sprite_id<E> } -> std::convertible_to<int>;
    } && anim::sprite_id<E> >= 0;

    struct Info { int sprite_id; uint32_t anim_id; };

    template <sprite_enum E>
    constexpr Info info(E e) { return { sprite_id<E>, static_cast<uint32_t>(e) }; }

    constexpr inline uint32_t sprite_count = ${idx};
    constexpr inline uint32_t total_anim_count = ${anim_count};

    // helper function to get the number of animations for a given sprite id
    inline uint32_t sprite_anim_count(const int _sprite_id) noexcept
    {
        switch (_sprite_id)
        {${sprite_anims_count_cases}
        }

        assert(false && \"Invalid sprite id\");
        return 0;
    }
}")

# make a command and target to run the texture packer and generate the header file on build

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