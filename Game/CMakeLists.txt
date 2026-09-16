project(Game)

add_executable(Game
    src/Game.cpp
    src/Main.cpp
)

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
