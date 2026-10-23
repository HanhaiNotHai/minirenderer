#include <minirenderer/platform/sdl_platform.hpp>

#include "color_copy.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <iostream>
#include <stdexcept>

using namespace minirenderer;

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void copies_rows_with_independent_padding() {
    const std::array<Rgba8, 6> source{{{255, 0, 0, 255},
                                       {0, 255, 0, 255},
                                       {99, 99, 99, 255},
                                       {0, 0, 255, 255},
                                       {255, 255, 255, 255},
                                       {99, 99, 99, 255}}};
    std::array<unsigned char, 32> destination;
    destination.fill(0xCC);
    detail::copy_color_rows(destination.data(), 16, {source.data(), 2, 2, 12});
    const std::array<unsigned char, 8> first{255, 0, 0, 255, 0, 255, 0, 255};
    const std::array<unsigned char, 8> second{0, 0, 255, 255, 255, 255, 255, 255};
    for (std::size_t i = 0; i < 8; ++i) {
        require(destination[i] == first[i], "first color row must retain byte order");
        require(destination[16 + i] == second[i], "source and destination strides differ");
        require(destination[8 + i] == 0xCC && destination[24 + i] == 0xCC,
                "row copies must not overwrite destination padding");
    }
}

SDL_WindowID window_id() {
    int count = 0;
    SDL_Window** windows = SDL_GetWindows(&count);
    const auto id = count == 1 ? SDL_GetWindowID(windows[0]) : 0;
    SDL_free(windows);
    require(id != 0, "test expects one platform-owned window");
    return id;
}

void push_window(SDL_EventType type, SDL_WindowID id) {
    SDL_Event event{};
    event.type = type;
    event.window.windowID = id;
    require(SDL_PushEvent(&event), "window event must enter real SDL queue");
}

void push_key(SDL_EventType type, SDL_WindowID id, SDL_Scancode code, bool repeat = false) {
    SDL_Event event{};
    event.type = type;
    event.key.windowID = id;
    event.key.scancode = code;
    event.key.repeat = repeat;
    require(SDL_PushEvent(&event), "keyboard event must enter real SDL queue");
}

void push_right_mouse(SDL_EventType type, SDL_WindowID id) {
    SDL_Event event{};
    event.type = type;
    event.button.windowID = id;
    event.button.button = SDL_BUTTON_RIGHT;
    require(SDL_PushEvent(&event), "mouse event must enter real SDL queue");
}

void polls_edges_and_clears_focus_loss() {
    SdlPlatform platform("input test", 2, 2, 20, 20);
    platform.poll_input();
    const auto id = window_id();
    push_window(SDL_EVENT_WINDOW_FOCUS_GAINED, id);
    push_key(SDL_EVENT_KEY_DOWN, id, SDL_SCANCODE_W);
    auto input = platform.poll_input();
    require(input.key(Key::W).down && input.key(Key::W).pressed && !input.key(Key::W).released,
            "initial press must set down and pressed");

    push_key(SDL_EVENT_KEY_DOWN, id, SDL_SCANCODE_W, true);
    input = platform.poll_input();
    require(input.key(Key::W).down && !input.key(Key::W).pressed,
            "repeat must not generate another press");

    push_key(SDL_EVENT_KEY_UP, id, SDL_SCANCODE_W);
    push_key(SDL_EVENT_KEY_DOWN, id, SDL_SCANCODE_F1);
    push_key(SDL_EVENT_KEY_UP, id, SDL_SCANCODE_F1);
    input = platform.poll_input();
    require(!input.key(Key::W).down && input.key(Key::W).released, "release must clear held state");
    require(!input.key(Key::F1).down && input.key(Key::F1).pressed && input.key(Key::F1).released,
            "a same-frame tap must retain both edges");

    push_key(SDL_EVENT_KEY_DOWN, id, SDL_SCANCODE_W);
    push_right_mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, id);
    input = platform.poll_input();
    require(input.right_mouse.down && input.right_mouse.pressed, "right mouse must be tracked");
    push_window(SDL_EVENT_WINDOW_FOCUS_LOST, id);
    push_key(SDL_EVENT_KEY_DOWN, id, SDL_SCANCODE_A);
    push_right_mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, id);
    input = platform.poll_input();
    require(!input.focused && !input.key(Key::W).down && !input.key(Key::A).down &&
                !input.right_mouse.down && input.mouse_dx == 0 && input.mouse_dy == 0,
            "unfocused input, including later queued events, must not stick");

    push_window(SDL_EVENT_WINDOW_FOCUS_GAINED, id);
    push_key(SDL_EVENT_KEY_DOWN, id, SDL_SCANCODE_ESCAPE);
    input = platform.poll_input();
    require(input.key(Key::Escape).pressed, "Esc must reach the application");
    push_window(SDL_EVENT_WINDOW_CLOSE_REQUESTED, id);
    require(platform.poll_input().quit_requested, "close must request application shutdown");
}

void presents_padded_view_and_rejects_invalid_views() {
    SdlPlatform platform("display test", 2, 2, 20, 20);
    const std::array<Rgba8, 6> pixels{
        {{255, 0, 0, 255}, {0, 255, 0, 255}, {}, {0, 0, 255, 255}, {255, 255, 255, 255}, {}}};
    platform.present({pixels.data(), 2, 2, 12});
    for (const auto view :
         {ColorView{nullptr, 2, 2, 8}, {pixels.data(), 1, 2, 8}, {pixels.data(), 2, 2, 4}}) {
        bool rejected = false;
        try {
            platform.present(view);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        require(rejected, "bad image views must be rejected before locking the texture");
    }
    platform.present({pixels.data(), 2, 2, 12});
    SDL_Event event{};
    event.type = SDL_EVENT_QUIT;
    require(SDL_PushEvent(&event), "quit event must enter queue");
    require(platform.poll_input().quit_requested, "global quit must request shutdown");
}
} // namespace

int main() {
    try {
        copies_rows_with_independent_padding();
        polls_edges_and_clears_focus_loss();
        presents_padded_view_and_rejects_invalid_views();
        std::cout << "sdl: all checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
