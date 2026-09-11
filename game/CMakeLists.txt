add_executable(MetalSlug)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

target_sources(MetalSlug PRIVATE ${GAME_SOURCES})
target_link_libraries(MetalSlug PRIVATE MetalSlugEngine)

set(asset_source_dir "${CMAKE_SOURCE_DIR}/assets")

if (WIN32)
    add_custom_command(TARGET MetalSlug POST_BUILD
            COMMAND powershell.exe -NoProfile -NonInteractive -Command
            "if (-not (Test-Path -LiteralPath '$<TARGET_FILE_DIR:MetalSlug>/assets'))  { [void](New-Item -ItemType Junction -Path '$<TARGET_FILE_DIR:MetalSlug>/assets'  -Value '${asset_source_dir}') };
             if (-not (Test-Path -LiteralPath '$<TARGET_FILE_DIR:MetalSlug>/texture_packer')) { [void](New-Item -ItemType Junction -Path '$<TARGET_FILE_DIR:MetalSlug>/texture_packer' -Value '${texture_packer_out_dir}') }"
            COMMENT "Linking assets and shaders (Windows junctions)"
            VERBATIM
    )
else()
    add_custom_command(TARGET MetalSlug POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:MetalSlug>"
            COMMAND bash -c "test -e '$<TARGET_FILE_DIR:MetalSlug>/assets' || ln -s '${asset_source_dir}' '$<TARGET_FILE_DIR:MetalSlug>/assets'"
            COMMAND bash -c "test -e '$<TARGET_FILE_DIR:MetalSlug>/texture_packer' || ln -s '${texture_packer_out_dir}' '$<TARGET_FILE_DIR:MetalSlug>/texture_packer'"
            COMMENT "Linking assets and shaders (symlinks)"
            VERBATIM
    )
endif()