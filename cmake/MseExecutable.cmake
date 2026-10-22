include_guard(GLOBAL)

function(add_mse_executable target)
    add_executable(${target})
    link_mimalloc(${target})

    file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
            ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

    target_sources(${target} PRIVATE ${GAME_SOURCES})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${target} PRIVATE MetalSlugEngine)
    target_precompile_headers(${target} PRIVATE ${CMAKE_SOURCE_DIR}/engine/include/mse/mse.hpp)

    mse_apply_flags(${target})


    snake_to_pascal_case(${target} pascal_name)
    set(main "${CMAKE_CURRENT_BINARY_DIR}/${target}_main.cpp")
    file(WRITE ${main} "
        #include \"${target}.hpp\"

        // #if !defined(NDEBUG) && defined(_MSC_VER)
        // #include <crtdbg.h>
        // #endif

        int main()
        {
            // #if !defined(NDEBUG) && defined(_MSC_VER)
            // _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
            // _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            // _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            // #endif

            ${pascal_name} game;
            game.setup();
            game.init();
            game.run();
            game.shutdown();
        }")

    target_sources(${target} PRIVATE ${main})



    set(asset_source_dir "${CMAKE_SOURCE_DIR}/assets")

    # make symlinks/junctions for assets and texture packer output in the build directory
    # that way the game can access them without needing to copy them over
    # copied from github.com/Misho12345/Boza (mine)
    if (WIN32)
        # junction because symlinks require admin privileges on Windows
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND powershell.exe -NoProfile -NonInteractive -Command
                "if (-not (Test-Path -LiteralPath '$<TARGET_FILE_DIR:${target}>/assets'))  { [void](New-Item -ItemType Junction -Path '$<TARGET_FILE_DIR:${target}>/assets'  -Value '${asset_source_dir}') };
             if (-not (Test-Path -LiteralPath '$<TARGET_FILE_DIR:${target}>/texture_packer')) { [void](New-Item -ItemType Junction -Path '$<TARGET_FILE_DIR:${target}>/texture_packer' -Value '${texture_packer_out_dir}') }"
                COMMENT "Creating junction for assets and texture packer output"
                VERBATIM
        )
    else()
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target}>"
                COMMAND bash -c "test -e '$<TARGET_FILE_DIR:${target}>/assets' || ln -s '${asset_source_dir}' '$<TARGET_FILE_DIR:${target}>/assets'"
                COMMAND bash -c "test -e '$<TARGET_FILE_DIR:${target}>/texture_packer' || ln -s '${texture_packer_out_dir}' '$<TARGET_FILE_DIR:${target}>/texture_packer'"
                COMMENT "Creating symlinks for assets and texture packer output"
                VERBATIM
        )
    endif()
endfunction()

