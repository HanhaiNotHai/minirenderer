add_executable(offline main.cpp ppm.cpp)
target_link_libraries(offline PRIVATE core)
target_include_directories(offline PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../common")

if(MR_BUILD_VIEWER AND NOT BUILD_TESTING)
    set_target_properties(offline PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
