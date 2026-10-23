#include "demo.hpp"

#include <minirenderer/platform/sdl_platform.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

int main() {
    try {
        minirenderer::Framebuffer framebuffer(minirenderer::demo::width,
                                              minirenderer::demo::height);
        minirenderer::SdlPlatform platform("minirenderer", framebuffer.width(),
                                           framebuffer.height(), 640, 640);
        while (true) {
            const auto input = platform.poll_input();
            if (input.quit_requested || input.key(minirenderer::Key::Escape).pressed) {
                break;
            }
            minirenderer::demo::render(framebuffer);
            platform.present(framebuffer.color_view());
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "viewer: " << error.what() << '\n';
        return 1;
    }
}
