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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -g)
  add_link_options(-fsanitize=address,undefined)
endif()

target_include_directories(Game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(Game PRIVATE cxx_std_20)

target_link_libraries(Game PRIVATE
    Engine
)
