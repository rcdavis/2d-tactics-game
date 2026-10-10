project(Engine)

add_library(Engine STATIC
    glad/src/gl.c
    src/Input/ActionMap.cpp
    src/Input/Input.cpp
    src/Renderer/OpenGL/GLBuffer.cpp
    src/Renderer/OpenGL/GLRenderDevice.cpp
    src/Renderer/OpenGL/GLShader.cpp
    src/Renderer/OpenGL/GLTexture.cpp
    src/Renderer/OpenGL/GLVertexArray.cpp
    src/Renderer/Buffer.cpp
    src/Renderer/Camera2D.cpp
    src/Renderer/IRenderDevice.cpp
    src/Renderer/Renderer2D.cpp
    src/Renderer/ShaderSystem.cpp
    src/Renderer/TextureSystem.cpp
    src/Tiles/TileMap.cpp
    src/Tiles/TileSystem.cpp
    src/Utils/Log.cpp
)

if(USE_SDL3_BACKEND)
    target_compile_definitions(Engine PUBLIC USE_SDL3_BACKEND)

    target_sources(Engine PRIVATE
        src/SDL/Platform.cpp
        src/SDL/SDLWindow.cpp
        stb/src/stb_image.c
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Engine PUBLIC
        DEBUG
        LOGGING_ENABLED
        ASSERTS_ENABLED
    )

    add_compile_options(-fsanitize=address,undefined -g)
    add_link_options(-fsanitize=address,undefined)
endif()

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/stb/include
)

target_compile_features(Engine PRIVATE cxx_std_20)

target_link_libraries(Engine PUBLIC
    spdlog::spdlog
    glm::glm
)

target_link_libraries(Engine PRIVATE
    SDL3::SDL3-static
    nlohmann_json::nlohmann_json
)
