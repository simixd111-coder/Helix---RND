# Helix RND — Architecture Diagram (Phase 1)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              APPLICATION                                     │
│  (C, C++, C#, Python, Rust, JS/WASM, Go, Java, Lua via bindings)           │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           HELIX C API (helix.h)                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐            │
│  │  Core       │ │  Window     │ │  Render     │ │  Math       │            │
│  │  boot/quit  │ │  make_win   │ │  make_*     │ │  vec/mat/   │            │
│  │  hx_last_   │ │  win_*      │ │  draw       │ │  quat       │            │
│  │  error()    │ │             │ │             │ │             │            │
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘            │
│         Opaque handles (HxWin, HxWorld, HxMesh, HxSkin, HxCam, ...)         │
│         Result codes: HxResult (HX_OK, HX_ERR_*)                            │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    ▼                 ▼                 ▼
┌─────────────────────────┐ ┌─────────────────────┐ ┌─────────────────────┐
│      PLATFORM LAYER     │ │    RENDER BACKENDS  │ │     MATH LIB        │
│  (no external deps)     │ │  (dynamic load)     │ │  (header-only)      │
├─────────────────────────┤ ├─────────────────────┤ ├─────────────────────┤
│  Win32 (Windows)        │ │  Vulkan             │ │  vec2/3/4           │
│  X11 + Wayland dlopen   │ │  OpenGL / Metal     │ │  mat3/4             │
│  Cocoa (macOS)          │ │  Software (CPU)     │ │  quat               │
│  Canvas (WASM)          │ │                     │ │  transform helpers  │
│  Headless (offscreen)   │ │  Shaders embedded   │ │                     │
│  Foreign window (HWND,  │ │  SPIR-V / GLSL      │ │  SIMD (optional)    │
│  NSView, wl_surface)    │ │                     │ │                     │
└─────────────────────────┘ └─────────────────────┘ └─────────────────────┘
                    │                 │                 │
                    └─────────────────┼─────────────────┘
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         STATIC LINKING / VENDORING                           │
│  • No Vulkan SDK, GLFW, SDL, GLEW, glad required                            │
│  • Vulkan/OpenGL loaders generated/embedded                                 │
│  • Shaders precompiled to SPIR-V / GLSL, embedded as byte arrays            │
│  • C/C++ runtime linked statically (/MT, -static-libstdc++)                 │
│  • Each language package bundles native binary (pip, cargo, npm, nuget...)  │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Backend Selection (Auto-fallback)

```
HX_GPU_AUTO (default) → Vulkan → OpenGL/Metal → Software
HX_GPU_VK             → Vulkan only (fail if unavailable)
HX_GPU_GL             → OpenGL/Metal only
HX_GPU_SOFT           → Software rasterizer only
```

## Repository Layout (Phase 1)

```
helix-rnd/
├── include/
│   └── helix.h              # Public C API (stable ABI)
├── src/
│   ├── core/                # Core engine (boot, error, handles)
│   ├── math/                # Header-only math library
│   ├── platform/            # Platform abstraction (window, input)
│   │   ├── win32/
│   │   ├── x11/
│   │   ├── wayland/
│   │   ├── cocoa/
│   │   ├── wasm/
│   │   └── headless/
│   └── render/              # Render backends
│       ├── vulkan/
│       ├── opengl/
│       ├── metal/
│       └── software/
├── tests/                   # Unit + image comparison tests
├── bindings/                # Language bindings (C++, C#, Python, Rust, JS, Go, Java, Lua)
├── scripts/                 # Build, package, install scripts
├── docs/
│   ├── VOCABULARY.md        # Token dictionary
│   ├── DEPENDENCIES.md      # Third-party licenses
│   └── DECISIONS.md         # Design decisions log
├── CMakeLists.txt
├── .github/workflows/ci.yml
└── LICENSE (MIT)
```

## Build & Test (Phase 1)

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -DHX_STATIC_RUNTIME=ON

# Build
cmake --build build --config Release

# Test
ctest --test-dir build --output-on-failure

# Run demo (headless triangle → PNG)
./build/helix_demo_headless
```