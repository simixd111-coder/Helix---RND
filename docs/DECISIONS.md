# Helix RND — Design Decisions Log

**Version:** 0.1.0 (Phase 1)
**Purpose:** Record architectural decisions with rationale for future reference.

---

## Decision Template

Each entry follows:
- **ID**: HX-YYYYMMDD-NNN
- **Title**: Short description
- **Status**: Proposed / Accepted / Superseded / Rejected
- **Context**: Why this decision was needed
- **Decision**: What was chosen
- **Consequences**: Trade-offs, follow-up work
- **Related**: Links to issues, PRs, other decisions

---

## Decisions

### HX-20261003-001: Pure C API with Opaque Handles
**Status:** Accepted
**Context:** Need stable ABI for bindings (C, C++, C#).
**Decision:** Public API is pure C99 (`helix.h`). All objects are opaque handles (`typedef struct HxWin HxWin;`). Functions use `hx_` prefix, types use `Hx` prefix, constants use `HX_` prefix.
**Consequences:**
- No C++ in public headers — enables direct FFI from any language
- Handle validation adds slight overhead (debug builds)
- C++ bindings are thin wrappers over C API
**Related:** Vocabulary doc, bindings architecture

### HX-20261003-002: Static Linking & Vendoring Only
**Status:** Accepted
**Context:** User must `choco install helix-rnd` / `dotnet add package HelixRND` and have it work without installing Vulkan SDK, GLFW, SDL, Visual C++ Redistributable, etc.
**Decision:**
- All third-party code vendored in `src/thirdparty/`
- C/C++ runtime linked statically (`/MT`, `-static-libstdc++ -static-libgcc`)
- Vulkan/OpenGL loaders generated/embedded at build time
- Shaders precompiled to SPIR-V/GLSL, embedded as byte arrays
- No dynamic dependencies except OS libraries (kernel32, user32, libc, libm, etc.)
**Consequences:**
- Larger binary size (~2-5 MB base)
- No "DLL hell" or version conflicts
- Build system more complex (code generation steps)
- Must track upstream security updates manually
**Related:** DEPENDENCIES.md, CMakeLists.txt

### HX-20261003-003: Backend Auto-Selection with Fallback
**Status:** Accepted
**Context:** Must work on any machine: high-end GPU, integrated graphics, headless CI, WebAssembly.
**Decision:** Default `HX_GPU_AUTO` tries Vulkan → OpenGL/Metal → Software. Environment variable `HX_GPU` can force: `VK`, `GL`, `METAL`, `SOFT`.
**Consequences:**
- Software renderer must be functional enough for basic demos/tests
- Backend detection at `hx_boot()` time
- Applications can query active backend via `hx_backend()`
**Related:** helix.h backend API, CI matrix

### HX-20261003-004: Platform Abstraction Without GLFW/SDL
**Status:** Accepted
**Context:** No external deps. Need windows + input on Windows, Linux (X11+Wayland), macOS, WASM.
**Decision:** Own platform layer in `src/platform/`:
- Win32 (Windows)
- X11 + Wayland (dlopen at runtime, Linux)
- Cocoa (macOS)
- Emscripten/Canvas (WASM)
- Headless (all platforms)
**Consequences:**
- More code to maintain (~3000 lines per platform)
- Full control over window behavior, HiDPI, input
- No GLFW version lag or bugs
- Foreign window embedding (HWND, NSView, wl_surface) supported via `hx_win_from_foreign()`
**Related:** Platform stubs in Phase 1

### HX-20261003-005: Vocabulary-Driven API Naming
**Status:** Accepted
**Context:** Consistent, memorable, cross-language API. Avoid "manager", "system", "controller" suffixes.
**Decision:** Tokens are short readable English words; compounds with `_` allowed (`double_sided`, `render_headless`). Pattern: `verb + object` with fixed verb set: `make`, `load`, `drop`, `add`, `draw`, `move`, `spin`, `size`, `look`, `set`, `get`, `snap`, `say`, `tween`, `on`. Documented in `docs/VOCABULARY.md`.
**Consequences:**
- API feels like a small language
- Easy to guess function names
- Bindings follow same pattern (`helix.boot()`, `hx::boot()`, `Hx.Boot()`)
- Refactoring vocabulary = breaking change (major version)
**Related:** VOCABULARY.md, helix.h

### HX-20261003-006: Math Library as Header-Only POD Types
**Status:** Accepted
**Context:** Math types passed by value across FFI boundary. No hidden allocations.
**Decision:** `HxVec2/3/4`, `HxMat4`, `HxQuat`, `HxColor` are plain structs in `helix.h`. Helper functions (`hx_mat4_mul`, `hx_quat_slerp`) in C API. SIMD optimizations internal.
**Consequences:**
- Zero-overhead FFI (memcpy-compatible)
- No alignment surprises
- C++ bindings can add operator overloads
- Large structs passed by value (16-64 bytes) — acceptable for math
**Related:** helix.h math section, math.cpp

### HX-20261003-007: Error Handling via HxResult + Thread-Local String
**Status:** Accepted
**Context:** C API needs rich errors without exceptions. Must work across threads.
**Decision:** Functions return `HxResult` (int32_t). `HX_OK = 0`, negative = error codes. `hx_last_error()` returns thread-local human-readable string.
**Consequences:**
- Caller must check return values
- Error strings not localized (English only)
- Thread-local storage adds TLS overhead (negligible)
- No error codes in hot paths (draw calls return void)
**Related:** helix.h error section, error.cpp

### HX-20261003-008: Phase-Gated Development
**Status:** Accepted
**Context:** Large project (10 phases). Need working, testable deliverable each phase.
**Decision:** Each phase produces:
- Compilable library + headers
- Unit tests passing
- At least one demo application
- CI green on Windows/Linux/macOS
- Documentation updated
**Consequences:**
- Phase 1: Architecture, C API, math, platform stubs, software renderer, headless triangle demo
- No "big bang" integration
- Early feedback on API usability
**Related:** Phase list in spec, CI workflow

### HX-20261003-009: Column-Major Matrices, Right-Handed Coordinates
**Status:** Accepted
**Context:** Match Vulkan/Metal/OpenGL conventions. Interop with glTF.
**Decision:** `HxMat4` is column-major (OpenGL/Vulkan/Metal native). Coordinate system: right-handed, Y up, Z forward (camera looks down -Z). Matches glTF.
**Consequences:**
- Direct upload to GPU uniform buffers
- `hx_cam_persp`/`hx_cam_ortho` produce column-major matrices
- Math helpers follow same convention
**Related:** math.cpp, camera API

### HX-20261003-010: Headless Mode as First-Class Citizen
**Status:** Accepted
**Context:** CI, testing, server-side rendering, thumbnail generation.
**Decision:** `HX_WIN_HEADLESS` flag creates window without native surface. `hx_render_headless()` renders directly to memory buffer. Demo produces PNG.
**Consequences:**
- Software renderer must support offscreen rendering
- No window system dependency for tests
- Enables image comparison tests in CI
**Related:** Headless demo, CI workflow

---

## New Decisions from Vocabulary Alignment (2026-10-03)

### HX-20261003-011: Strict Verb Set for API Functions
**Status:** Accepted
**Context:** Original API had inconsistent verbs (`world_add`, `world_drop`, `cam_look`, `cam_persp`, `lamp_sun_dir`, `buffer_write`, `mat4_mul`, `quat_slerp`, etc.) making it hard to guess names.
**Decision:** Fixed verb set: `make`, `load`, `drop`, `add`, `draw`, `move`, `spin`, `size`, `look`, `set`, `get`, `snap`, `say`, `tween`, `on`. All functions follow `hx_<verb>_<object>`.
**Consequences:**
- Major renaming of existing functions (see RENAME_MAPPING.md)
- `hx_world_add` → `hx_add_mesh`, `hx_world_drop` → `hx_drop_mesh`
- `hx_cam_look` → `hx_look`, `hx_cam_persp` → `hx_set_cam_persp`
- `hx_lamp_sun_dir` → `hx_set_lamp_dir`, `hx_buffer_write` → `hx_write_buffer`
- `hx_mat4_mul` → `hx_mul_mat4`, `hx_quat_slerp` → `hx_slerp_quat`
- Math helpers use consistent prefixes: `make_`, `mul_`, `inverse_`, `transpose_`, `slerp_`, `rotate_`
**Related:** RENAME_MAPPING.md, VOCABULARY.md, helix.h

### HX-20261003-012: Unified Event Callback System
**Status:** Accepted
**Context:** Six separate callback setters (`hx_win_set_key_cb`, `hx_win_set_mouse_btn_cb`, etc.) were verbose and inconsistent.
**Decision:** Single `hx_on(win, mask, callback, user)` with `HxEvent` struct and `HxEventType` bitmask. Replaces all individual setters.
**Consequences:**
- Simpler API, easier to bind
- Event struct with union for type-safe payloads
- Callbacks return `bool` (handled) for potential event filtering
**Related:** helix.h events section, VOCABULARY.md

### HX-20261003-013: Get/Set Prefix for All Property Accessors
**Status:** Accepted
**Context:** Mixed naming: `hx_win_size` (getter), `hx_win_set_size` (setter), `hx_win_dt` (getter), `hx_cam_view` (getter).
**Decision:** All getters use `hx_get_<object>_<property>`, all setters use `hx_set_<object>_<property>`.
**Consequences:**
- `hx_win_size` → `hx_get_win_size`, `hx_win_dt` → `hx_get_win_dt`
- `hx_win_set_title` → `hx_set_win_title` (already consistent)
- `hx_cam_view` → `hx_get_cam_view`, `hx_cam_persp` → `hx_set_cam_persp`
- `hx_key_state` → `hx_get_key_state`, `hx_mouse_btn` → `hx_get_mouse_btn`
**Related:** RENAME_MAPPING.md, VOCABULARY.md

### HX-20261003-014: Separate GPU Request (HxGpu) from Active Backend (HxBackend)
**Status:** Accepted
**Context:** Original API used single enum for both what user wants and what's actually running.
**Decision:** Two distinct enums:
- `HxGpu` / `HX_GPU_*`: User request at boot (`AUTO`, `VK`, `GL`, `METAL`, `SOFT`)
- `HxBackend` / `HX_BACKEND_*`: Actual backend in use (queried via `hx_get_backend()`)
**Consequences:**
- Clearer semantics, no confusion
- Environment variable `HELIX_GPU` maps to `HxGpu` values
- `hx_boot()` takes `HxCfg*` with `gpu` field
**Related:** helix.h boot config, VOCABULARY.md

### HX-20261003-015: Safe Double-Drop Returns Error Code
**Status:** Accepted
**Context:** Calling `drop` twice on same handle could crash or be undefined behavior.
**Decision:** All `hx_drop_*` functions return `HxResult`. Double drop returns `HX_ERR_ALREADY_DROPPED` (safe, no crash). `hx_quit()` destroys all remaining handles automatically.
**Consequences:**
- Callers can check return value if needed
- Deterministic resource management encouraged
- No crashes from accidental double-free
**Related:** helix.h error codes, VOCABULARY.md ownership section

### HX-20261003-016: Clear Gamepad Constant Names
**Status:** Accepted
**Context:** Abbreviated constants (`LB`, `RB`, `LT`, `RT`, `LS`, `RS`) were ambiguous.
**Decision:** Full descriptive names:
- `HX_PAD_BUMP_L` / `HX_PAD_BUMP_R` (bumpers)
- `HX_PAD_TRIGGER_L` / `HX_PAD_TRIGGER_R` (triggers)
- `HX_PAD_STICK_L` / `HX_PAD_STICK_R` (stick presses)
**Consequences:**
- Self-documenting, no need to memorize abbreviations
- Consistent with `pad_` prefix for all pad constants
**Related:** helix.h gamepad section, VOCABULARY.md

### HX-20261003-017: Skin = Material, Rig = Skeletal Animation
**Status:** Accepted
**Context:** Term "skin" used for both material and skeletal skinning (confusing).
**Decision:** 
- `HxSkin` / `skin` = material/appearance only
- `HxRig` / `rig` = skeletal rig (bones, hierarchy)
- `HxClip` / `clip` = animation clip
- `anim` used only as concept word, not in API
**Consequences:**
- Clear separation of concerns
- `hx_make_skin_pbr` for materials, `hx_load_rig` for skeletons
**Related:** helix.h skin/rig/clip, VOCABULARY.md

### HX-20261003-018: Picture (CPU) vs Texture (GPU) Distinction
**Status:** Accepted
**Context:** Original API had only `HxTex` for both GPU textures and CPU images.
**Decision:** Two distinct types:
- `HxTex` / `tex` = GPU texture resource (created via `hx_make_tex`, `hx_load_tex`)
- `HxPic` / `pic` = CPU-side image (loaded via `hx_load_pic`, saved via `hx_save_pic`)
**Consequences:**
- Clear ownership: `pic` for loading/processing, `tex` for rendering
- `hx_save_pic` enables screenshots, `hx_snap_win` captures window to PNG
**Related:** helix.h texture/picture, VOCABULARY.md

### HX-20261003-019: Language Style Convention Table
**Status:** Accepted
**Context:** Bindings need consistent naming per language idioms.
**Decision:** Documented in VOCABULARY.md:
- C/C++: `snake_case`
- C#: `PascalCase`
**Consequences:**
- Binding generators can follow this automatically
- Cross-language consistency
**Related:** VOCABULARY.md, bindings architecture

### HX-20261003-020: Open Source Governance Files
**Status:** Accepted
**Context:** Project is open source (MIT) from day one. Needs standard governance files.
**Decision:** Added:
- `LICENSE` (MIT)
- `CONTRIBUTING.md` (build, test, style, PR process, vocabulary rules)
- `CODE_OF_CONDUCT.md` (Contributor Covenant 2.1)
- `SECURITY.md` (vulnerability reporting)
- `GOVERNANCE.md` (core team, decision making, releases)
- `CHANGELOG.md` (Keep a Changelog format)
- `.github/ISSUE_TEMPLATE/` (bug, feature, question)
- `.github/PULL_REQUEST_TEMPLATE.md`
- `.clang-format` (LLVM-based, 4-space, Allman braces)
- SPDX headers on all source files
**Consequences:**
- Professional open source project setup
- Clear contribution process
- Automated style enforcement
**Related:** Root directory files, .github/

---

## Superseded / Rejected

*None yet — Phase 1 just started.*

---

## Changelog

| Date | Decision IDs | Notes |
|------|--------------|-------|
| 2026-10-03 | HX-20261003-001 through -020 | Phase 1 initial decisions + vocabulary alignment |