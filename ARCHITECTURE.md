# Helix RND Architecture

## Current scope

Helix RND exposes a C11 API in include/helix.h and implements it in C++20. The software renderer supports headless solid and textured triangles, RGBA8 atlases, and PNG capture. Vulkan support is optional and incomplete. Visible-window drawing, PBR, text, skeletal animation, and post-processing are not implemented.

Windows and Linux/X11 are the native platform targets. Headless windows work without a native window system. Wayland, WASM, and macOS are outside the current support scope.

## Layers

- Application: calls the public C API from C, C++, or the C# wrapper.
- Public API: validates requests and exposes opaque handles and result codes.
- Core: lifecycle, resources, handles, errors, cameras, meshes, and scene entities.
- Math: vectors, quaternions, and column-major 4x4 matrices.
- Platform: window lifecycle, input, events, monotonic timing, and sleeping.
- Renderer: software rasterization; optional Vulkan code is still partial.
- Dependencies: vendored C/C++ libraries under src/thirdparty.

## Frame loop and timing

A window loop calls hx_tick, updates and renders, then calls hx_show. hx_tick polls events and updates hx_get_win_dt. hx_set_win_fps_limit configures the maximum tick rate per window; zero means uncapped. Headless windows can use this cap. For native windows, enabling VSync bypasses the software limiter.

The timer uses std::chrono::steady_clock. Delta time includes frame pacing and work between ticks. hx_get_win_time returns monotonic elapsed time from the platform timer.

## Matrix convention

HxMat4 stores values column-major as m[column][row] and is used with column vectors. hx_mul_mat4(a, b, out) computes a times b. The output may alias either input. hx_make_mat4_trs composes translation, rotation, then scale (T x R x S).

## Repository modules

- include/: public C API.
- src/core/: engine lifecycle, handles, resources, and scene objects.
- src/math/: math types and operations.
- src/platform/: windows, input, event handling, and timing; native adapters are in platform-specific subdirectories.
- src/render/software/: software renderer and rasterizer.
- src/thirdparty/: vendored dependencies.
- tests/: API, math, platform, renderer, and resource tests.
- demos/: headless triangle and visual smoke example.
- packaging/ and pkg/: package definitions and distribution artifacts.
- docs/: design decisions, dependency notes, publishing instructions, and API vocabulary.

## Build and test

Configure with CMake using HX_BUILD_TESTS=ON and HX_BUILD_HEADLESS_DEMO=ON, build the selected configuration, then run ctest --test-dir build --output-on-failure. See README.md and CONTRIBUTING.md for platform-specific commands.
