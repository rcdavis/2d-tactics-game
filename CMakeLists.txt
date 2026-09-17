cmake_minimum_required(VERSION 3.28)

project(2dTacticsGame LANGUAGES CXX)

option(USE_SDL3_BACKEND "Use SDL3 as the backend" ON)

include(cmake/dependencies.cmake)

add_subdirectory(Engine)
add_subdirectory(Game)
