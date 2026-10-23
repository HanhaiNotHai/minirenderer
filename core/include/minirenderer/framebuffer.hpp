#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace minirenderer {

struct Rgba8 {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};
static_assert(sizeof(Rgba8) == 4);

// Borrows rows until its owner is assigned or destroyed; stride includes row padding.
struct ColorView {
    const Rgba8* pixels;
    int width;
    int height;
    std::size_t stride_bytes;
};

class Framebuffer {
public:
    Framebuffer(int width, int height);
    Framebuffer(const Framebuffer&) = default;
    Framebuffer& operator=(const Framebuffer& other);
    Framebuffer(Framebuffer&&) = delete;
    Framebuffer& operator=(Framebuffer&&) = delete;

    int width() const noexcept;
    int height() const noexcept;

    std::span<Rgba8> colors() noexcept;
    std::span<const Rgba8> colors() const noexcept;
    std::span<float> depths() noexcept;
    std::span<const float> depths() const noexcept;

    void clear(Rgba8 color, float depth);
    ColorView color_view() const noexcept;

private:
    int width_;
    int height_;
    std::vector<Rgba8> colors_;
    std::vector<float> depths_;
};

} // namespace minirenderer
