add_library(sdl STATIC src/sdl_platform.cpp)
target_include_directories(sdl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(sdl PUBLIC core PRIVATE SDL3::SDL3)
