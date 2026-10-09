# Helix RND — Dependencies & Licenses

**Version:** 3.0.0
**Dependencies:** stb_truetype added in this release for TTF font rasterization.
**Policy:** Third-party code is vendored. The software backend has no GPU runtime requirement; Vulkan mode dynamically loads the operating-system Vulkan loader and a compatible driver.

---

## Vendored Dependencies

| Library | Purpose | License | Source | Status |
|---------|---------|---------|--------|--------|
| **stb_image** | PNG, JPEG, BMP, TGA, PNM and other CPU image decoding | MIT / public domain | `src/thirdparty/stb/stb_image.h` | Vendored |
| **stb_truetype** | TTF font rasterization for text rendering | MIT / public domain | `src/thirdparty/stb/stb_truetype.h` | Vendored and used by software renderer |
| **Vulkan-Headers** | Vulkan API declarations | Apache-2.0 OR MIT | `src/thirdparty/vulkan/` | Vendored, not yet used by renderer |
| **Volk** | Dynamic Vulkan function loader | MIT | `src/thirdparty/volk/` | Vendored and used for Vulkan device/buffer operations |
| **cgltf** | glTF 2.0 / GLB parsing | MIT | `src/thirdparty/cgltf.h` | Vendored and used for static mesh import |

---

## Planned Dependencies (Future Phases)

| Library | Purpose | License | Vendoring Method |
|---------|---------|---------|------------------|
| **stb_image_write** | PNG output for demos/tests | MIT | Single header, embed in `src/thirdparty/stb/` |
| **fast_obj** | OBJ loading | MIT | Single header, embed in `src/thirdparty/fast_obj/` |
| **glad** | OpenGL loader (generated) | MIT | Generate at build, embed in `src/thirdparty/glad/` |
| **metal-cpp** | Metal C++ bindings | Apache-2.0 | Vendored from Apple repo |
| **emscripten** | WASM toolchain (build-time only) | MIT | System package, not vendored |
| **miniz** | ZIP/app packaging | MIT | Single file, embed in `src/thirdparty/miniz/` |
| **zstd** | Compression (assets) | BSD-3 | Vendored, static link |

---

## System Dependencies (Build-time only)

| Platform | Packages | Purpose |
|----------|----------|---------|
| Windows | Visual Studio 2022 + Windows SDK | Compiler, linker, headers |
| Linux | `build-essential`, `ninja-build`, `libx11-dev` | Compiler, X11 headers |
| WASM | Emscripten SDK | WASM toolchain |

---

## Runtime Dependencies

The software backend needs no graphics runtime. Vulkan mode requires a system Vulkan loader and compatible driver, loaded dynamically; the Vulkan SDK is not required to build.
- Frame timing and sleeping use the C++ standard library (`std::chrono`, `std::this_thread`); no additional dependency is needed.
- C/C++ runtime can be linked statically (`/MT` on Windows, `-static-libstdc++ -static-libgcc` on Linux)
- No GLFW, SDL, GLEW or runtime SDK installation is required
- GPU drivers are provided by the OS/IHV and are not bundled
- Software rendering remains available as a fallback

---

## License Compliance

- All vendored code retains original license headers
- `LICENSE` file at repo root (MIT)
- This document (`docs/DEPENDENCIES.md`) tracks all third-party code
- Binary distributions include `LICENSE` and `THIRD_PARTY_LICENSES.txt`

---

## Adding a New Dependency

1. Verify license is permissive (MIT, Apache-2.0, BSD, zlib, Public Domain)
2. Prefer single-header or single-file libraries
3. Vendor in `src/thirdparty/<name>/`
4. Add entry to this document
5. Update `THIRD_PARTY_LICENSES.txt` generation script
6. Document in `docs/DECISIONS.md` why it was chosen
