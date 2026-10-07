# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.5.0] - 2026-10-07

### Added
- Per-window FPS cap API: `hx_set_win_fps_limit` and `hx_get_win_fps_limit`; `0` disables the cap.
- Windows visual smoke demo for interactive renderer checks.

### Fixed
- Correct column-major matrix multiplication order and support aliased output.
- Implement monotonic platform timing and sleeping so delta time and frame pacing work.

### Changed
- Reuse software renderer buffers between same-size frames to reduce repeated allocation and visible flicker.
- Native VSync takes priority over the software FPS cap; headless windows can use the cap.

## [2.0.0] - 2026-10-07

### Added
- Headless software rendering for solid/textured triangles, RGBA8 atlas composition, scalar tweens, and PNG capture.
- Windows and Linux release builds with automated tests; macOS is outside the supported release matrix.

### Known limitations
- PBR, visible-window drawing, shaders, lighting, text, skeletal animation, and post-process effects remain unsupported.

### Changed
- Set library and package metadata to version 2.0.0; release targets are Windows and Linux.

### Fixed
- Resolve static component cycles for GNU/Linux linkers and correct CI artifact and NuGet publishing steps.
- Include the NuGet README and native Windows DLL in the package.

- Initial project structure and CMake build system
- Public C API (`include/helix.h`) with opaque handles
- Math library (vec2/3/4, mat4, quat, color)
- Boot configuration with GPU backend selection
- Window abstraction (headless working, platform stubs)
- Unified event system (`hx_on`)
- Input handling (keyboard, mouse, gamepad)
- World/Scene management
- Mesh creation and transform operations
- Skin (material) system
- Camera (3D perspective, 2D orthographic)
- Light (sun, point, spot)
- Texture (GPU) and Picture (CPU) APIs
- Shader loading (embedded bytecode)
- GPU Buffer API
- Draw commands (world, mesh, custom passes)
- Math helpers with consistent naming
- 2D/Animation/Post-process stubs
- Headless rendering to memory buffer
- Unit tests (math, handles, error, platform)
- Headless triangle demo (produces triangle.png)
- Vocabulary documentation
- Open source governance files (LICENSE, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, GOVERNANCE)

### Changed
- N/A

### Deprecated
- N/A

### Removed
- N/A

### Fixed
- N/A

### Security
- N/A

## [1.0.0] - 2026-10-04

### Added
- Real Vulkan device discovery and adapter selection
- Vulkan device/buffer creation with host-visible memory, read/write and queue fill
- CPU image loading and tracking
- Static glTF/GLB mesh import via vendored `cgltf`
- Sprite sheet + frame animation API
- Resource accounting and double-drop safety
- Cross-platform validation under Windows and Debian/WSL

### Changed
- Project version bumped to 1.0.0
- README and dependency documentation updated to reflect the implemented release baseline

### Notes
- Vulkan rendering pipelines, swapchain presentation and full GPU draw path remain future work.
- This release is a valid 1.0.0 engine foundation with real GPU resource access and asset import support.

---

## Release Template

### [X.Y.Z] - YYYY-MM-DD

### Added
- New features

### Changed
- Changes in existing functionality

### Deprecated
- Soon-to-be removed features

### Removed
- Removed features

### Fixed
- Bug fixes

### Security
- Security fixes
