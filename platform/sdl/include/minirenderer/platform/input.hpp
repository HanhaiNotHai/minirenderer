#pragma once

#include <array>
#include <cstddef>

namespace minirenderer {

struct ButtonState {
    bool down = false;
    bool pressed = false;
    bool released = false;
};

enum class Key { W, A, S, D, Escape, F1, Count };

struct InputState {
    std::array<ButtonState, static_cast<std::size_t>(Key::Count)> keys{};
    ButtonState right_mouse;
    float mouse_dx = 0;
    float mouse_dy = 0;
    bool focused = false;
    bool quit_requested = false;

    const ButtonState& key(Key code) const noexcept { return keys[static_cast<std::size_t>(code)]; }
};

} // namespace minirenderer
