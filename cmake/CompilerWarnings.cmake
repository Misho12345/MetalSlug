include_guard(GLOBAL)

# copied from github.com/Misho12345/Boza (mine)
function(enable_warnings target)
    target_compile_definitions(${target} PUBLIC _CRT_SECURE_NO_WARNINGS)

    if (MSVC)
        target_compile_options(${target} PRIVATE
                /W4
                /WX
                /permissive-
                /Zc:__cplusplus
                /sdl
                /wd4267    # conversion
                /wd4146    # - on unsigned for overflow
                /wd4702    # unreachable code
                /wd4065    # switch with 'default' but no 'case'
                /wd4251    # DLL-interface warning
                /wd5050    # modules compatibility
                /wd4324    # structure was padded due to alignment specifier
                /w14263    # member function doesn't override any base class virtual
                /w14265    # class has virtual functions but no virtual destructor
        )

        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.30")
            target_compile_options(${target} PRIVATE /wd5105)
        endif ()
    else ()
        # Common GCC/Clang warnings
        target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Werror
                -Wno-missing-field-initializers
                -Wcast-align
                -Wold-style-cast
                -Wshadow
                -Wnull-dereference
                -Wdouble-promotion
        )

        # GCC-specific warnings
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target} PRIVATE
                    -Wlogical-op
                    -Wduplicated-cond
                    -Wduplicated-branches
                    -Wnull-dereference
                    -Wdouble-promotion
            )
        endif ()

        # Clang-specific warnings
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target} PRIVATE
                    -Wmost
                    -Wextra-semi
                    -Wcomma
                    -Wnon-virtual-dtor
            )
        endif ()
    endif ()
endfunction()