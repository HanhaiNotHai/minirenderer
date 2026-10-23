#include <minirenderer/renderer.hpp>

namespace minirenderer {

bool Renderer::draw_point(Framebuffer& target, int x, int y, Rgba8 color) const noexcept {
    if (x < 0 || y < 0 || x >= target.width() || y >= target.height()) {
        return false;
    }
    const auto index = static_cast<std::size_t>(y) * static_cast<std::size_t>(target.width()) +
                       static_cast<std::size_t>(x);
    target.colors()[index] = color;
    return true;
}

} // namespace minirenderer
