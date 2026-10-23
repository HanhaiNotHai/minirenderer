#pragma once

#include <minirenderer/framebuffer.hpp>
#include <minirenderer/renderer.hpp>

#include <limits>

namespace minirenderer::demo {

inline constexpr int width = 64;
inline constexpr int height = 64;

inline void render(Framebuffer& target) {
    target.clear({0, 0, 0, 255}, std::numeric_limits<float>::infinity());
    const Renderer renderer;
    constexpr Rgba8 white{255, 255, 255, 255};
    renderer.draw_point(target, 7, 3, white);
    renderer.draw_point(target, 12, 37, white);
    renderer.draw_point(target, 62, 53, white);
}

} // namespace minirenderer::demo
