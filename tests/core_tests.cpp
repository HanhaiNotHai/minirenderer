#include <minirenderer/framebuffer.hpp>
#include <minirenderer/renderer.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

using namespace minirenderer;

namespace {
bool fail_next_allocation = false;
}

// Inject one real allocation failure into vector's copy operation.
void* operator new(std::size_t size) {
    if (std::exchange(fail_next_allocation, false)) {
        throw std::bad_alloc();
    }
    if (void* memory = std::malloc(size == 0 ? 1 : size)) {
        return memory;
    }
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool matches(Rgba8 pixel, int r, int g, int b, int a = 255) {
    return pixel.r == r && pixel.g == g && pixel.b == b && pixel.a == a;
}

void rejects_invalid_dimensions() {
    for (const auto [width, height] : {std::pair{0, 4}, {4, 0}, {-1, 4}, {4, -1}}) {
        bool rejected = false;
        try {
            Framebuffer framebuffer(width, height);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        require(rejected, "non-positive framebuffer size must be rejected");
    }
    bool rejected = false;
    try {
        Framebuffer framebuffer(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    } catch (const std::length_error&) {
        rejected = true;
    }
    require(rejected, "unrepresentable storage size must fail before allocation");
}

void initializes_and_clears_both_buffers() {
    Framebuffer framebuffer(3, 2);
    require(framebuffer.colors().size() == 6, "all color pixels must be allocated");
    require(framebuffer.depths().size() == 6, "every pixel needs depth storage");
    for (const auto pixel : framebuffer.colors()) {
        require(matches(pixel, 0, 0, 0), "initial color must be opaque black");
    }
    for (const auto depth : framebuffer.depths()) {
        require(std::isinf(depth) && depth > 0, "initial depth must be positive infinity");
    }
    framebuffer.clear({17, 34, 51, 255}, 0.75F);
    for (const auto pixel : framebuffer.colors()) {
        require(matches(pixel, 17, 34, 51), "clear must overwrite every color");
    }
    for (const auto depth : framebuffer.depths()) {
        require(depth == 0.75F, "clear must overwrite every depth");
    }
    const Framebuffer& readonly = framebuffer;
    const auto view = readonly.color_view();
    require(view.width == 3 && view.height == 2 && view.stride_bytes == 12,
            "color view must describe its rows in bytes");
    require(matches(view.pixels[5], 17, 34, 51), "const view must see cleared pixels");
}

void draws_top_left_coordinates_and_rejects_outside_points() {
    Framebuffer framebuffer(3, 2);
    Renderer renderer;
    require(renderer.draw_point(framebuffer, 0, 0, {255, 0, 0, 255}), "top-left is valid");
    require(renderer.draw_point(framebuffer, 2, 1, {0, 255, 0, 255}), "bottom-right is valid");
    require(matches(framebuffer.colors()[0], 255, 0, 0), "origin must be first pixel");
    require(matches(framebuffer.colors()[5], 0, 255, 0), "coordinates must index y * width + x");
    for (const auto [x, y] : {std::pair{-1, 0}, {0, -1}, {3, 0}, {0, 2}, {3, 1}}) {
        require(!renderer.draw_point(framebuffer, x, y, {255, 255, 255, 255}),
                "out-of-bounds point must report failure");
    }
    for (std::size_t index = 1; index < 5; ++index) {
        require(matches(framebuffer.colors()[index], 0, 0, 0),
                "rejected points must not wrap into another row");
    }
    require(matches(framebuffer.colors()[0], 255, 0, 0) &&
                matches(framebuffer.colors()[5], 0, 255, 0),
            "rejected points must not clamp over edge pixels");
    for (const auto depth : framebuffer.depths()) {
        require(std::isinf(depth), "point drawing must leave depth unchanged");
    }
}

void failed_copy_assignment_preserves_the_target() {
    Framebuffer target(1, 1);
    target.clear({17, 34, 51, 255}, 0.25F);
    Framebuffer source(2, 2);
    bool failed = false;
    fail_next_allocation = true;
    try {
        target = source;
    } catch (const std::bad_alloc&) {
        failed = true;
    }
    require(failed, "copy assignment must exercise an allocation failure");
    require(target.width() == 1 && target.height() == 1 && target.colors().size() == 1 &&
                target.depths().size() == 1,
            "failed assignment must preserve matching dimensions and storage");
    require(matches(target.colors()[0], 17, 34, 51) && target.depths()[0] == 0.25F,
            "failed assignment must preserve original pixels and depth");
    require(!Renderer{}.draw_point(target, 1, 1, {255, 255, 255, 255}),
            "failed assignment must not allow writes beyond original storage");
    source.clear({255, 255, 255, 255}, 0.5F);
    target = source;
    require(target.width() == 2 && target.height() == 2 && target.colors().size() == 4 &&
                matches(target.colors()[3], 255, 255, 255) && target.depths()[3] == 0.5F,
            "successful assignment must copy both buffers and dimensions");
}
} // namespace

int main() {
    try {
        rejects_invalid_dimensions();
        initializes_and_clears_both_buffers();
        draws_top_left_coordinates_and_rejects_outside_points();
        failed_copy_assignment_preserves_the_target();
        std::cout << "core: all checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
