project(AssetBuilder)

add_executable(AssetBuilder
    src/AssetBuilder.cpp
    src/Main.cpp
)

target_include_directories(AssetBuilder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(AssetBuilder PRIVATE cxx_std_20)

target_link_libraries(AssetBuilder PRIVATE
    CLI11::CLI11
    pugixml
)
