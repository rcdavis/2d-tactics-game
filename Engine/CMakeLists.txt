project(Engine)

add_library(Engine STATIC
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

target_link_libraries(Engine PUBLIC
    spdlog::spdlog
    SDL3::SDL3-static
)
