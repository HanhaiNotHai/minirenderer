#include <minirenderer/platform/sdl_platform.hpp>

#include "color_copy.hpp"

#include <SDL3/SDL.h>

#include <optional>
#include <stdexcept>
#include <string>

namespace minirenderer {
namespace {
void check_sdl(bool success, const char* operation) {
    if (!success) {
        throw std::runtime_error(std::string(operation) + ": " + SDL_GetError());
    }
}

struct VideoSubsystem {
    VideoSubsystem() { check_sdl(SDL_InitSubSystem(SDL_INIT_VIDEO), "SDL_InitSubSystem"); }
    ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); }
};

std::optional<Key> map_key(SDL_Scancode scancode) {
    switch (scancode) {
    case SDL_SCANCODE_W:
        return Key::W;
    case SDL_SCANCODE_A:
        return Key::A;
    case SDL_SCANCODE_S:
        return Key::S;
    case SDL_SCANCODE_D:
        return Key::D;
    case SDL_SCANCODE_ESCAPE:
        return Key::Escape;
    case SDL_SCANCODE_F1:
        return Key::F1;
    default:
        return std::nullopt;
    }
}

void update_button(ButtonState& state, bool down) {
    if (down && !state.down) {
        state.pressed = true;
    }
    if (!down && state.down) {
        state.released = true;
    }
    state.down = down;
}
} // namespace

struct SdlPlatform::Impl {
    VideoSubsystem video;
    int width;
    int height;
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window{nullptr, SDL_DestroyWindow};
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer{nullptr,
                                                                           SDL_DestroyRenderer};
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{nullptr,
                                                                        SDL_DestroyTexture};
    InputState input;
    bool relative_mouse = false;

    Impl(std::string_view title, int render_width, int render_height, int window_width,
         int window_height)
        : width(render_width), height(render_height) {
        if (width <= 0 || height <= 0 || window_width <= 0 || window_height <= 0) {
            throw std::invalid_argument("render and window dimensions must be positive");
        }
        const std::string window_title(title);
        window.reset(SDL_CreateWindow(window_title.c_str(), window_width, window_height,
                                      SDL_WINDOW_RESIZABLE));
        check_sdl(window != nullptr, "SDL_CreateWindow");
        renderer.reset(SDL_CreateRenderer(window.get(), nullptr));
        check_sdl(renderer != nullptr, "SDL_CreateRenderer");
        texture.reset(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA32,
                                        SDL_TEXTUREACCESS_STREAMING, width, height));
        check_sdl(texture != nullptr, "SDL_CreateTexture");
        check_sdl(SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_NONE),
                  "SDL_SetTextureBlendMode");
        check_sdl(SDL_SetTextureScaleMode(texture.get(), SDL_SCALEMODE_NEAREST),
                  "SDL_SetTextureScaleMode");
        check_sdl(SDL_SetRenderLogicalPresentation(renderer.get(), width, height,
                                                   SDL_LOGICAL_PRESENTATION_LETTERBOX),
                  "SDL_SetRenderLogicalPresentation");
        check_sdl(SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255), "SDL_SetRenderDrawColor");
        input.focused = (SDL_GetWindowFlags(window.get()) & SDL_WINDOW_INPUT_FOCUS) != 0;
    }

    void set_relative_mouse_mode(bool enabled) {
        if (relative_mouse != enabled) {
            check_sdl(SDL_SetWindowRelativeMouseMode(window.get(), enabled),
                      "SDL_SetWindowRelativeMouseMode");
            relative_mouse = enabled;
            input.mouse_dx = input.mouse_dy = 0;
        }
    }

    InputState poll_input() {
        for (auto& key : input.keys) {
            key.pressed = key.released = false;
        }
        input.right_mouse.pressed = input.right_mouse.released = false;
        input.mouse_dx = input.mouse_dy = 0;
        const auto id = SDL_GetWindowID(window.get());
        bool focus_changed = false;
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                input.quit_requested = true;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID == id) {
                    input.quit_requested = true;
                }
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                if (event.window.windowID == id) {
                    const bool quit_requested = input.quit_requested;
                    input = {};
                    input.quit_requested = quit_requested;
                    set_relative_mouse_mode(false);
                    focus_changed = true;
                }
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                if (event.window.windowID == id) {
                    input.focused = true;
                    focus_changed = true;
                }
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                if (input.focused && event.key.windowID == id && !event.key.repeat) {
                    if (const auto key = map_key(event.key.scancode)) {
                        update_button(input.keys[static_cast<std::size_t>(*key)],
                                      event.type == SDL_EVENT_KEY_DOWN);
                    }
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (input.focused && event.button.windowID == id &&
                    event.button.button == SDL_BUTTON_RIGHT) {
                    update_button(input.right_mouse, event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                }
                break;
            default:
                break;
            }
        }
        float dx = 0, dy = 0;
        SDL_GetRelativeMouseState(&dx, &dy);
        if (input.focused && !focus_changed) {
            input.mouse_dx = dx;
            input.mouse_dy = dy;
        }
        return input;
    }

    void present(ColorView image) {
        detail::validate_color_view(image);
        if (image.width != width || image.height != height) {
            throw std::invalid_argument("image size must match the fixed render resolution");
        }
        void* pixels = nullptr;
        int pitch = 0;
        check_sdl(SDL_LockTexture(texture.get(), nullptr, &pixels, &pitch), "SDL_LockTexture");
        {
            struct Unlock {
                SDL_Texture* texture;
                ~Unlock() { SDL_UnlockTexture(texture); }
            } unlock{texture.get()};
            if (pitch <= 0) {
                throw std::runtime_error("SDL returned an invalid texture pitch");
            }
            detail::copy_color_rows(pixels, static_cast<std::size_t>(pitch), image);
        }
        check_sdl(SDL_RenderClear(renderer.get()), "SDL_RenderClear");
        check_sdl(SDL_RenderTexture(renderer.get(), texture.get(), nullptr, nullptr),
                  "SDL_RenderTexture");
        check_sdl(SDL_RenderPresent(renderer.get()), "SDL_RenderPresent");
    }
};

SdlPlatform::SdlPlatform(std::string_view title, int render_width, int render_height,
                         int window_width, int window_height)
    : impl_(std::make_unique<Impl>(title, render_width, render_height, window_width,
                                   window_height)) {}

SdlPlatform::~SdlPlatform() = default;
InputState SdlPlatform::poll_input() { return impl_->poll_input(); }
void SdlPlatform::present(ColorView image) { impl_->present(image); }
void SdlPlatform::set_relative_mouse_mode(bool enabled) { impl_->set_relative_mouse_mode(enabled); }

} // namespace minirenderer
