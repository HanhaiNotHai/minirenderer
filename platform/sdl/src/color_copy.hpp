#pragma once

#include <minirenderer/framebuffer.hpp>

#include <cstring>
#include <limits>
#include <stdexcept>

namespace minirenderer::detail {

inline void validate_color_view(ColorView image) {
    if (!image.pixels || image.width <= 0 || image.height <= 0) {
        throw std::invalid_argument("color view must have pixels and positive dimensions");
    }
    const auto row_bytes = static_cast<std::size_t>(image.width) * sizeof(Rgba8);
    const auto last_row = static_cast<std::size_t>(image.height - 1);
    if (image.stride_bytes < row_bytes ||
        (last_row > 0 &&
         image.stride_bytes > (std::numeric_limits<std::size_t>::max() - row_bytes) / last_row)) {
        throw std::invalid_argument("invalid color view stride");
    }
}

inline void copy_color_rows(void* destination, std::size_t destination_stride, ColorView image) {
    validate_color_view(image);
    const auto row_bytes = static_cast<std::size_t>(image.width) * sizeof(Rgba8);
    if (!destination || destination_stride < row_bytes) {
        throw std::invalid_argument("invalid destination row stride");
    }
    const auto* source = reinterpret_cast<const unsigned char*>(image.pixels);
    auto* target = static_cast<unsigned char*>(destination);
    for (int y = 0; y < image.height; ++y) {
        const auto row = static_cast<std::size_t>(y);
        std::memcpy(target + row * destination_stride, source + row * image.stride_bytes,
                    row_bytes);
    }
}

} // namespace minirenderer::detail
