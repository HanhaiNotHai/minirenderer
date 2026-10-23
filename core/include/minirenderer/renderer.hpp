#pragma once

#include <minirenderer/framebuffer.hpp>

namespace minirenderer {

class Renderer {
public:
    // Returns false for coordinates outside the target; never clamps or wraps.
    bool draw_point(Framebuffer& target, int x, int y, Rgba8 color) const noexcept;
};

} // namespace minirenderer
