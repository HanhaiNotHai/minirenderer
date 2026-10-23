add_executable(core_tests core_tests.cpp)
target_link_libraries(core_tests PRIVATE core)
add_test(NAME core COMMAND core_tests)

if(TARGET offline)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME offline COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_offline.py" "$<TARGET_FILE:offline>")
endif()

if(TARGET sdl)
    add_executable(sdl_tests sdl_tests.cpp)
    target_link_libraries(sdl_tests PRIVATE sdl SDL3::SDL3)
    target_include_directories(sdl_tests PRIVATE "${PROJECT_SOURCE_DIR}/platform/sdl/src")
    add_test(NAME sdl COMMAND sdl_tests)
    set_tests_properties(sdl PROPERTIES
        ENVIRONMENT "SDL_VIDEO_DRIVER=dummy;SDL_RENDER_DRIVER=software")
endif()
