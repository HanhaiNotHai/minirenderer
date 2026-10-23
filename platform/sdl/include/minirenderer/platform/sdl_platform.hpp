#pragma once

#include <minirenderer/framebuffer.hpp>
#include <minirenderer/platform/input.hpp>

#include <memory>
#include <string_view>

namespace minirenderer {

// Create, use and destroy on the main thread. All SDL resources stay private.
class SdlPlatform {
public:
    SdlPlatform(std::string_view title, int render_width, int render_height, int window_width,
                int window_height);
    ~SdlPlatform();
    SdlPlatform(const SdlPlatform&) = delete;
    SdlPlatform& operator=(const SdlPlatform&) = delete;
    SdlPlatform(SdlPlatform&&) = delete;
    SdlPlatform& operator=(SdlPlatform&&) = delete;

    InputState poll_input();
    void present(ColorView image);
    void set_relative_mouse_mode(bool enabled);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace minirenderer
