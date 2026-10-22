include_guard(GLOBAL)

if (MSE_USE_MIMALLOC)
    find_package(mimalloc CONFIG REQUIRED)
endif ()

function(link_mimalloc target)
    if (NOT MSE_USE_MIMALLOC)
        return()
    endif ()

    target_link_libraries(${target} PRIVATE mimalloc)

    if (MSVC)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_options(${target} PRIVATE /INCLUDE:mi_version) # x64
        else ()
            target_link_options(${target} PRIVATE /INCLUDE:_mi_version) # x86
        endif ()
    endif ()
endfunction()