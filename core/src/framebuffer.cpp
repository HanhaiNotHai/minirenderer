#include <minirenderer/framebuffer.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace minirenderer {
namespace {
std::size_t checked_pixel_count(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("framebuffer dimensions must be positive");
    }
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    if (h > std::numeric_limits<std::size_t>::max() / w) {
        throw std::length_error("framebuffer dimensions overflow");
    }
    const auto count = w * h;
    if (count > std::vector<Rgba8>().max_size() || count > std::vector<float>().max_size()) {
        throw std::length_error("framebuffer exceeds maximum storage size");
    }
    return count;
}
} // namespace

Framebuffer::Framebuffer(int width, int height)
    : width_(width), height_(height), colors_(checked_pixel_count(width, height)),
      depths_(colors_.size(), std::numeric_limits<float>::infinity()) {}

Framebuffer& Framebuffer::operator=(const Framebuffer& other) {
    if (this != &other) {
        // Publish dimensions and both buffers only after all allocations succeed.
        Framebuffer copy(other);
        std::swap(width_, copy.width_);
        std::swap(height_, copy.height_);
        colors_.swap(copy.colors_);
        depths_.swap(copy.depths_);
    }
    return *this;
}

int Framebuffer::width() const noexcept { return width_; }
int Framebuffer::height() const noexcept { return height_; }

std::span<Rgba8> Framebuffer::colors() noexcept { return colors_; }
std::span<const Rgba8> Framebuffer::colors() const noexcept { return colors_; }
std::span<float> Framebuffer::depths() noexcept { return depths_; }
std::span<const float> Framebuffer::depths() const noexcept { return depths_; }

void Framebuffer::clear(Rgba8 color, float depth) {
    std::fill(colors_.begin(), colors_.end(), color);
    std::fill(depths_.begin(), depths_.end(), depth);
}

ColorView Framebuffer::color_view() const noexcept {
    return {colors_.data(), width_, height_, static_cast<std::size_t>(width_) * sizeof(Rgba8)};
}

} // namespace minirenderer
