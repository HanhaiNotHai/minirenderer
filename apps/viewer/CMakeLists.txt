add_executable(viewer main.cpp)
target_link_libraries(viewer PRIVATE core sdl)
target_include_directories(viewer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../common")
