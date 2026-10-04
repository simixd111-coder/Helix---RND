# Helix RND

**A modular C API for building custom game engines, with C++ internals and a software fallback.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)]()

---

## Overview

Helix RND ("Helix") is a C API and C++ library intended as a modular foundation for custom game engines and games.

```bash
# Python (planned)
pip install helix-rnd

# C# (planned)
dotnet add package HelixRND

# Rust (planned)
cargo add helix-rnd

# Node.js (planned)
npm install helix-rnd

# Go (planned)
go get github.com/helix-rnd/helix/go

# Java/Maven (planned)
<dependency>
  <groupId>dev.helix</groupId>
  <artifactId>helix-rnd</artifactId>
  <version>0.1.0</version>
</dependency>

# Lua (planned)
luarocks install helix-rnd
```

The software build needs no graphics SDK. Vulkan headers and Volk are vendored; Vulkan mode loads the system driver dynamically. No GLFW or SDL is required.

---

## Status: Phase 1 — Foundation (In Progress)

### Implemented and tested
- C99 public API (`include/helix.h`) with C++20 internals
- Software/headless renderer and headless triangle demo
- Vulkan device enumeration/selection and real Vulkan buffers, read/write, and queue fill
- CPU/GPU memory accounting, double-drop protection and shutdown cleanup
- CPU image loading to RGBA8, static glTF/GLB primitive import, grid sprite sheets and frame animation
- Windows and Debian/WSL builds; tests compile and link a C source file against the library

### Not implemented yet
- Vulkan draw pipelines, swapchain presentation and GPU-backed `hx_draw_*`; drawing remains software/headless
- glTF node transforms, materials, skins and skeletal animation
- Sprite draw/batching, atlas packing, text, particles and tilemaps
- Audio, ECS, editor, native-window event integration on all platforms, OpenGL, Metal, WASM and bindings

### 📋 Planned (not started)
See [Roadmap](#roadmap) below.

---

## Quick Start (C99) — Headless Window and Resource Tracking
```c
#include <helix.h>

int main(void) {
    HxCfg cfg = {0};
    cfg.gpu = HX_GPU_SOFT;
    cfg.headless = true;
    if (hx_boot(&cfg) != HX_OK) return 1;

    HxWin win = hx_make_win(320, 240, "My Engine", HX_WIN_HEADLESS);
    if (win == HX_NULL_HANDLE) { hx_quit(); return 2; }

    HxMemoryStats stats = {0};
    hx_get_memory_stats(&stats);
    if (stats.live_resources == 0) { hx_drop_win(win); hx_quit(); return 3; }
    hx_drop_win(win);
    hx_quit();
    hx_get_memory_stats(&stats);
    return stats.live_resources == 0 && stats.cpu_bytes == 0 ? 0 : 4;
}
```

---

## Quick Start (Python) — Planned API

```python
import helix as hx

cfg = hx.Cfg(gpu=hx.GPU.SOFT)
hx.boot(cfg)

win = hx.make_win(1280, 720, "Cube")
world = hx.make_world()
skin = hx.make_skin(hx.ORANGE)
cube = hx.make_cube(skin)
hx.add_mesh(world, cube, skin, None)

sun = hx.make_lamp(hx.LAMP.SUN, hx.WHITE, 1.0)
hx.set_lamp_dir(sun, hx.Vec3(0, -1, -1))
hx.add_mesh(world, sun, None, None)

cam = hx.make_cam3d()
hx.look(cam, at=hx.Vec3(0,0,0), from=hx.Vec3(3,2,5), up=hx.Vec3(0,1,0))
hx.set_cam_persp(cam, fov=60, aspect=1280/720, near=0.1, far=100)

while hx.get_win_alive(win):
    hx.tick(win)
    hx.spin_mesh(cube, 0, 1.0 * hx.get_win_dt(win), 0)
    hx.draw_world(win, world, cam)
    hx.show(win)

hx.quit()
```

---

## Build from Source

### Prerequisites

| Platform | Requirements |
|----------|--------------|
| Windows | Visual Studio 2022 + Windows SDK, Ninja, CMake 3.20+ |
| Linux | `build-essential`, `ninja-build`, `cmake`, `libx11-dev`, `libwayland-dev`, `libxkbcommon-dev` |
| macOS | Xcode Command Line Tools, `ninja`, `cmake` (Homebrew) |

### Build

```bash
git clone https://github.com/<your-org>/helix-rnd.git
cd helix-rnd

# Configure (Release, static runtime, Vulkan device/buffers and software fallback)
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DHX_STATIC_RUNTIME=ON \
  -DHX_BUILD_TESTS=ON \
  -DHX_BUILD_HEADLESS_DEMO=ON \
  -DHX_BACKEND_VULKAN=ON \
  -DHX_BACKEND_SOFTWARE=ON

# Build
cmake --build build --config Release --parallel

# Test
cd build && ctest --output-on-failure

# Run headless software demo (produces triangle.png)
./helix_demo_headless
```

---

## Vulkan Device Selection

Set at boot time via `HxCfg.gpu` or environment variable:

```c
HxCfg cfg = {0};
cfg.gpu = HX_GPU_VK;
cfg.gpu_preference = HX_GPU_PREFERENCE_HIGH_PERFORMANCE;
cfg.gpu_device_index = HX_GPU_DEVICE_DEFAULT;
hx_boot(&cfg);
```

Vulkan mode discovers/selects a real adapter and supports GPU buffers and queue fill operations. Graphics pipelines and presentation are still in progress; `hx_draw_*` does not render through Vulkan yet. `HX_GPU_AUTO` falls back to software if no compatible device exists. An explicit `HX_GPU_VK` request returns an error instead of silently claiming GPU rendering.

Query devices with `hx_get_gpu_devices()` and select a policy with `hx_pick_gpu_device()`.

Environment variable (`HX_GPU`) can also request a backend when configuration uses auto:
```bash
export HX_GPU=VK     # Vulkan
export HX_GPU=SOFT   # Software rasterizer
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION (any language)                │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    HELIX C API (helix.h)                     │
│  Core │ Window │ Render │ Math │ Input │ Assets │ Error     │
└─────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
┌───────────────┐    ┌─────────────────┐    ┌───────────────┐
│   PLATFORM    │    │  RENDER BACKENDS │    │    MATH       │
│  (per OS)     │    │ Vulkan buffers   │    │  C++ math     │
├───────────────┤    ├─────────────────┤    ├───────────────┤
│ Win32         │    │ GPU draw planned │    │ vec2/3/4      │
│ X11/Wayland   │    │ Software draw    │    │ mat4          │
│ Cocoa         │    │ OpenGL planned   │    │ quat          │
│ WASM planned  │    │ Metal planned    │    │ transform     │
│ Headless      │    │                  │    │ SIMD (opt)    │
└───────────────┘    └─────────────────┘    └───────────────┘
```

**Backend Selection:** Vulkan device buffers or software rendering. OpenGL and Metal are not implemented yet.

---

## Project Structure

```
helix-rnd/
├── include/helix.h          # Public C API (stable ABI)
├── src/
│   ├── core/                # Boot, resources, camera, glTF, GPU device
│   ├── math/                # Math library
│   ├── platform/            # Window + input (per OS)
│   │   ├── win32/
│   │   ├── x11/
│   │   ├── wayland/
│   │   ├── cocoa/
│   │   ├── wasm/
│   │   └── headless/
│   ├── render/software/     # Headless software triangle renderer
│   └── thirdparty/          # stb_image, cgltf, Volk, Vulkan-Headers
├── tests/                   # Unit + image comparison tests
├── demos/                   # Demo applications
├── bindings/                # Language bindings (planned)
├── scripts/                 # Build, package, install scripts (planned)
├── docs/
│   ├── VOCABULARY.md        # Token dictionary
│   ├── DEPENDENCIES.md      # Third-party licenses
│   └── DECISIONS.md         # Design decisions log
├── CMakeLists.txt
├── .github/workflows/ci.yml
├── LICENSE (MIT)
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── SECURITY.md
├── GOVERNANCE.md
└── CHANGELOG.md
```

---

## Roadmap

| Phase | Focus | Status |
|-------|-------|--------|
| **1** | C API, math, resources, headless software renderer | ✅ Foundation |
| **2** | Vulkan device selection, tracked buffers, queue fill, glTF mesh import, sprite frame animation | ✅ Core slice; GPU draw pending |
| 3 | Vulkan graphics pipelines, swapchain and GPU-backed drawing | 📋 In progress |
| 4 | Blender materials, node transforms and skeletal animation | 📋 Planned |
| 5 | Audio, ECS, project templates and editor tools | 📋 Planned |
| 6 | OpenGL, Metal, WebGPU and language bindings | 📋 Planned |
| 9 | Bindings with embedded binaries (pip, cargo, npm, nuget, ...) | 📋 Planned |
| 10 | CLI (`helix doctor/new/run/demo/update`), packaging, publishing | 📋 Planned |

---

## Documentation

- [Vocabulary / Token Dictionary](docs/VOCABULARY.md)
- [Dependencies & Licenses](docs/DEPENDENCIES.md)
- [Design Decisions](docs/DECISIONS.md)
- [Architecture Diagram](ARCHITECTURE.md)

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for:
- How to build and run tests
- Code style (C++20, clang-format)
- How to add a new token respecting the vocabulary
- PR process

**Good first issues:**
- Implement Win32 window backend
- Implement X11 window backend
- Add more math helper functions
- Improve software rasterizer (triangle interpolation)
- Add image comparison test infrastructure

---

## Open Source Governance

| File | Purpose |
|------|---------|
| [LICENSE](LICENSE) | MIT License |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Contribution guide |
| [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) | Contributor Covenant |
| [SECURITY.md](SECURITY.md) | Vulnerability reporting |
| [GOVERNANCE.md](GOVERNANCE.md) | Decision-making process |
| [CHANGELOG.md](CHANGELOG.md) | Keep a Changelog format |
| [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md) | Vendorized dependencies |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Design decisions log |

---

## License

MIT License — see [LICENSE](LICENSE) for details.

Third-party licenses in [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md).

---

## Contact

- GitHub: https://github.com/<your-org>/helix-rnd
- Issues: https://github.com/<your-org>/helix-rnd/issues
- Discussions: https://github.com/<your-org>/helix-rnd/discussions