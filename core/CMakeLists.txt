add_library(core STATIC
    src/framebuffer.cpp
    src/renderer.cpp
)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
