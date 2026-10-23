# Minimal Renderer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Build the approved 64×64 three-pixel demo through viewer and offline paths.

**Architecture:** Application-owned RGBA32/float framebuffer, SDL-independent core, private SDL adapter, two thin applications sharing demo data.

**Tech Stack:** C++20, CMake 3.25+, existing SDL 3.4.18; CTest and Python standard library for tests.

**Spec:** docs/superpowers/specs/2026-10-23-minimal-renderer-design.md

## Global Constraints

- Keep targets core/sdl/viewer/offline, existing SDL submodule and staged changes.
- 64×64, top-left origin, black background, white (7,3), (12,37), (62,53), alpha 255.
- Default viewer; explicit offline; viewer OFF must configure without SDL.
- 640×640 initial window, RGBA32 streaming upload with independent strides, nearest letterbox.
- No camera, lines, triangles or depth test in this increment.

## Review Focus

- Invalid framebuffer dimensions and overflow must fail before allocation (Task 1).
- Negative/off-edge coordinates must not overwrite another row (Task 1).
- Offline output failure must produce nonzero status (Task 2).
- Padded source rows must upload correctly; bad view dimensions/stride rejected (Task 3).
- Key repeats, same-frame tap and focus loss must not leave stuck input (Task 3).

### Task 1: Core framebuffer and point renderer

**Files:** core/CMakeLists.txt, core/include/minirenderer/{framebuffer,renderer}.hpp, core/src/{framebuffer,renderer}.cpp, tests/core_tests.cpp.

**Interfaces:** Produces Rgba8, ColorView, Framebuffer(int,int), width/height, colors/depths spans, clear(Rgba8,float), color_view(), Renderer::draw_point(Framebuffer&,int,int,Rgba8) const noexcept -> bool.

- [x] Write core behavior tests for invalid/overflow dimensions, clear/depth, top-left row indexing, edge/negative rejection, unchanged depth.
- [x] Run the test against missing implementation, then minimal stubs if necessary, and observe failure.
- [x] Implement minimal owning framebuffer and point renderer; run all core tests.

### Task 2: Build paths, shared demo, and offline output

**Files:** root CMakeLists.txt, CMakePresets.json, apps/common/demo.hpp, apps/offline/{CMakeLists.txt,main.cpp,ppm.hpp,ppm.cpp}, tests/{CMakeLists.txt,test_offline.py}.

**Interfaces:** Consumes Task 1; produces render_demo(Framebuffer&), write_ppm(path,ColorView), offline [output.ppm], MR_BUILD_VIEWER/MR_BUILD_OFFLINE and BUILD_TESTING options.

- [x] Write offline executable tests asserting literal PPM header, all RGB bytes with exactly three white positions, alternate path and error exits.
- [x] Run against missing executable and observe failure.
- [x] Implement shared demo, checked PPM writer and offline CLI; wire target-based CMake with optional tests.
- [x] Configure offline-only, build, run core and offline tests; check no SDL cache/targets are introduced.

### Task 3: SDL adapter, viewer, documentation and full verification

**Files:** platform/sdl/{CMakeLists.txt,include/minirenderer/platform/{input,sdl_platform}.hpp,src/sdl_platform.cpp}, apps/viewer/{CMakeLists.txt,main.cpp}, tests/sdl_tests.cpp, README.md.

**Interfaces:** Consumes ColorView and render_demo; produces InputState, SdlPlatform(title,render_width,render_height,window_width,window_height), poll_input(), present(ColorView), set_relative_mouse_mode(bool).

- [x] Write SDL integration tests using dummy/software and SDL_PushEvent, covering padded-color display, invalid views, press/repeat/tap/release/focus loss/quit and resource recreation; observe failure before implementation.
- [x] Implement RAII SDL ownership, per-frame input, pitch-aware upload and thin viewer with Esc/close handling.
- [x] Build fresh default configuration: viewer exists, offline does not; explicitly build offline.
- [x] Build test preset and run complete CTest suite; run headless offline and viewer smoke checks.
- [x] Add Chinese README with build/run/test commands and dependency boundaries; review all changes with a fresh reviewer and address material findings.

## Execution record

No automatic git commits: current index contains user work. Work in the supplied workspace; maintain this checklist and verification notes without altering staging.

Core/offline verification: `ctest --test-dir build-offline --output-on-failure` passed 2/2 tests, including four offline CLI/image checks.

Final verification:
- Default Debug build produced viewer and no offline executable; explicit offline target then built successfully.
- Offline-only configuration has no SDL directory or targets.
- Full CTest suite: 3/3 passed (core, offline, SDL dummy/software).
- WSLg viewer launched outside the restricted display sandbox and exited with status 0 after a shutdown signal.
- Independent review found one important issue: copy assignment could publish dimensions before allocation succeeded. A real bad_alloc regression failed first, then passed with copy-and-swap; complete suite passed after the fix.
- All owned C++ files pass the repository clang-format configuration. Existing user staging and SDL source remained untouched.
