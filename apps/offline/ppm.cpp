#include "ppm.hpp"

#include <fstream>
#include <stdexcept>

namespace minirenderer {

void write_ppm(const std::filesystem::path& path, ColorView image) {
    if (!image.pixels || image.width <= 0 || image.height <= 0 ||
        image.stride_bytes < static_cast<std::size_t>(image.width) * sizeof(Rgba8)) {
        throw std::invalid_argument("invalid PPM image view");
    }
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot open output file: " + path.string());
    }
    output.exceptions(std::ios::badbit | std::ios::failbit);
    output << "P6\n" << image.width << ' ' << image.height << "\n255\n";
    const auto* bytes = reinterpret_cast<const unsigned char*>(image.pixels);
    for (int y = 0; y < image.height; ++y) {
        const auto* row = bytes + static_cast<std::size_t>(y) * image.stride_bytes;
        for (int x = 0; x < image.width; ++x) {
            output.write(reinterpret_cast<const char*>(row + static_cast<std::size_t>(x) * 4), 3);
        }
    }
    output.close();
}

} // namespace minirenderer
