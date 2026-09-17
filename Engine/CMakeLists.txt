project(Engine)

add_library(Engine STATIC
    glad/src/gl.c
    src/OpenGL/GLRenderDevice.cpp
    src/SDL/Platform.cpp
    src/SDL/SDLWindow.cpp
    src/Utils/Log.cpp
)

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
)

target_compile_features(Engine PRIVATE cxx_std_20)

target_link_libraries(Engine PUBLIC
    spdlog::spdlog
)

target_link_libraries(Engine PRIVATE
    SDL3::SDL3-static
)
