project(Game)

add_executable(Game
    src/Game.cpp
    src/Main.cpp
)

# Copy textures to the build directory
file(GLOB_RECURSE TEXTURE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/res/textures/*.png)
set(COPIED_TEXTURES "")

foreach(TEXTURE_FILE ${TEXTURE_FILES})
    get_filename_component(TEXTURE_NAME ${TEXTURE_FILE} NAME)
    set(DEST_TEXTURE_FILE ${CMAKE_BINARY_DIR}/Game/res/textures/${TEXTURE_NAME})

    add_custom_command(
        OUTPUT ${DEST_TEXTURE_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TEXTURE_FILE} ${DEST_TEXTURE_FILE}
        DEPENDS ${TEXTURE_FILE}
        COMMENT "Copying texture: ${TEXTURE_NAME}"
    )

    list(APPEND COPIED_TEXTURES ${DEST_TEXTURE_FILE})
endforeach()

add_custom_target(CopyTextures ALL DEPENDS ${COPIED_TEXTURES})
add_dependencies(Game CopyTextures)

# Copy shaders to the build directory
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/res/shaders/*.vs"
    "${CMAKE_SOURCE_DIR}/res/shaders/*.fs"
)
set(COPIED_SHADERS "")

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(DEST_SHADER_FILE ${CMAKE_BINARY_DIR}/Game/res/shaders/${SHADER_NAME})

    add_custom_command(
        OUTPUT ${DEST_SHADER_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_FILE} ${DEST_SHADER_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Copying shader: ${SHADER_NAME}"
    )

    list(APPEND COPIED_SHADERS ${DEST_SHADER_FILE})
endforeach()

add_custom_target(CopyShaders ALL DEPENDS ${COPIED_SHADERS})
add_dependencies(Game CopyShaders)

file(GLOB_RECURSE TILEMAP_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/res/tilemaps/*.tsx"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/tilemaps/*.tmx"
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/Game/build_assets.stamp
    COMMAND $<TARGET_FILE:AssetBuilder>
        --input ${CMAKE_SOURCE_DIR}/res
        --output ${CMAKE_BINARY_DIR}/Game/res
        --generated ${CMAKE_BINARY_DIR}/generated
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/Game/build_assets.stamp
    DEPENDS ${TEXTURE_FILES} ${TILEMAP_FILES} ${SHADER_FILES} $<TARGET_FILE:AssetBuilder>
    COMMENT "Building Assets..."
    VERBATIM
)

add_custom_target(BuildAssets ALL DEPENDS ${CMAKE_BINARY_DIR}/Game/build_assets.stamp)
add_dependencies(Game BuildAssets)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -g)
  add_link_options(-fsanitize=address,undefined)
endif()

target_include_directories(Game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
)

target_compile_features(Game PRIVATE cxx_std_20)

target_link_libraries(Game PRIVATE
    Engine
)
