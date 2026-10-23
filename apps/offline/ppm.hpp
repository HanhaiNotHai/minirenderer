#pragma once

#include <minirenderer/framebuffer.hpp>

#include <filesystem>

namespace minirenderer {
void write_ppm(const std::filesystem::path& path, ColorView image);
} // namespace minirenderer
