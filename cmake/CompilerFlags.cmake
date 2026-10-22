include_guard(GLOBAL)

if (MSVC AND MSE_ENABLE_ASAN)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
    string(REGEX REPLACE "/RTC[1su]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif ()

# copied from github.com/Misho12345/Boza (mine)
function(mse_apply_flags target)
    target_compile_definitions(${target} PUBLIC _CRT_SECURE_NO_WARNINGS)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CLANG ON)

        if (MSVC)
            set(CLANG_CL ON)
            set(CLANG_GNU OFF)
        else ()
            set(CLANG_CL OFF)
            set(CLANG_GNU ON)
        endif ()
    else ()
        set(CLANG OFF)
    endif ()


    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(GNU ON)
    else ()
        set(GNU OFF)
    endif ()

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
                /w44365    # implicit sign conversion
        )
    else ()
        # Common GCC/Clang warnings
        target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Werror
                -Wno-missing-field-initializers
                -Wshadow
                -Wnull-dereference
                #                -Wdouble-promotion
                #                -Wsign-conversion
        )

        # GCC-specific warnings
        if (GNU)
            target_compile_options(${target} PRIVATE
                    -Wlogical-op
                    -Wduplicated-cond
                    -Wduplicated-branches
                    -Wnull-dereference
                    -Wcast-align=strict
            )
        endif ()

        # Clang-specific warnings
        if (CLANG)
            target_compile_options(${target} PRIVATE
                    -Wextra-semi
                    -Wcomma
                    -Wnon-virtual-dtor
                    -Wcast-align
            )
        endif ()
    endif ()


    if (MSE_ENABLE_ASAN)
        if (CLANG_CL)
            target_compile_options(${target} PRIVATE -fsanitize=address)
        elseif (MSVC)
            target_compile_options(${target} PRIVATE /fsanitize=address)
            target_link_options(${target} PRIVATE /INCREMENTAL:NO)
        else ()
            # GCC/GNU Clang
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif ()
    endif ()

    if (MSE_ENABLE_UBSAN)
        if (CLANG OR GNU)
            target_compile_options(${target} PRIVATE -fsanitize=undefined -fsanitize-trap=undefined)
            if (NOT CLANG_CL)
                target_link_options(${target} PRIVATE -fsanitize=undefined)
            endif ()
        else ()
            message(WARNING "UBSan is not supported for MSVC")
        endif ()
    endif ()
endfunction()