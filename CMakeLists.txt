cmake_minimum_required(VERSION 3.25)
project(minirenderer VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MR_BUILD_VIEWER "Build the SDL viewer" ON)
option(MR_BUILD_OFFLINE "Build the offline renderer" ON)
option(BUILD_TESTING "Build minirenderer tests" OFF)
include(CTest)

add_subdirectory(core)

if(MR_BUILD_VIEWER)
    add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)
    add_subdirectory(platform/sdl)
    add_subdirectory(apps/viewer)
endif()

if(MR_BUILD_OFFLINE)
    add_subdirectory(apps/offline)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
