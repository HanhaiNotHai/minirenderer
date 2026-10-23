#include "demo.hpp"
#include "ppm.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "usage: offline [output.ppm]\n";
        return 1;
    }
    try {
        minirenderer::Framebuffer framebuffer(minirenderer::demo::width,
                                              minirenderer::demo::height);
        minirenderer::demo::render(framebuffer);
        minirenderer::write_ppm(argc == 2 ? argv[1] : "framebuffer.ppm", framebuffer.color_view());
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "offline: " << error.what() << '\n';
        return 1;
    }
}
