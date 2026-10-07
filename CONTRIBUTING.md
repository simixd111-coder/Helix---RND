# Contributing to Helix RND

Thank you for your interest in contributing! This document explains how to build, test, and submit changes.

## Quick Start

### Prerequisites

| Platform | Requirements |
|----------|--------------|
| Windows | Visual Studio 2022 + Windows SDK, Ninja, CMake 3.20+ |
| Linux | `build-essential`, `ninja-build`, `cmake` |
| macOS | No está en la matriz de plataformas soportadas actualmente. |

### Build & Test

```bash
# Clone
git clone https://github.com/<your-org>/helix-rnd.git
cd helix-rnd

# Configure
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DHX_STATIC_RUNTIME=ON \
  -DHX_BUILD_TESTS=ON \
  -DHX_BUILD_HEADLESS_DEMO=ON \
  -DHX_BACKEND_SOFTWARE=ON

# Build
cmake --build build --config Release --parallel

# Run tests
cd build && ctest --output-on-failure

# Run headless demo (produces triangle.png)
./helix_demo_headless
```

## Code Style

### C/C++ (Core Engine)
- **Standard**: C++20 for implementation, C11 for the public API (`helix.h`)
- **Formatting**: `clang-format` (config in `.clang-format` at repo root)
- **Naming**: Follow the [Vocabulary](docs/VOCABULARY.md) strictly
  - Functions: `hx_<verb>_<object>` (e.g., `hx_make_win`, `hx_spin_mesh`)
  - Types: `Hx<Type>` (e.g., `HxWin`, `HxVec3`)
  - Constants: `HX_<NAME>` (e.g., `HX_OK`, `HX_WIN_FULLSCREEN`)
- **Headers**: Every source file must start with SPDX license identifier:
  ```c
  // SPDX-License-Identifier: MIT
  ```

### Adding a New Token

1. **Check the Vocabulary** — Ensure the concept doesn't already exist
2. **Pick the right verb** — Use only: `make`, `load`, `drop`, `add`, `draw`, `move`, `spin`, `size`, `look`, `set`, `get`, `snap`, `say`, `tween`, `on`
3. **Name it** — `hx_<verb>_<object>` in C, update `docs/VOCABULARY.md`
4. **Add to `helix.h`** — Function declaration + any new types/constants
5. **Implement** — Add to appropriate `.cpp` file
6. **Test** — Add unit test in `tests/`
7. **Update docs** — `VOCABULARY.md`, `README.md` if user-facing

## Pull Request Process

1. **Fork** the repository
2. **Create a feature branch** — `git checkout -b feat/my-feature`
3. **Make changes** — Follow code style, add tests
4. **Run tests locally** — `ctest --output-on-failure`
5. **Format code** — `clang-format -i` on changed files
6. **Commit** — Use conventional commits: `feat: add hx_move_mesh for mesh translation`
7. **Push** — `git push origin feat/my-feature`
8. **Open PR** — Fill out the PR template

### PR Requirements

- [ ] All tests pass (`ctest --output-on-failure`)
- [ ] Code formatted with `clang-format`
- [ ] New tokens documented in `docs/VOCABULARY.md`
- [ ] `helix.h` and `VOCABULARY.md` stay in sync
- [ ] No new warnings (compile with `-Wall -Wextra`)
- [ ] SPDX header on new files

## Reporting Issues

Use the issue templates:
- **Bug Report** — For crashes, incorrect behavior, regressions
- **Feature Request** — For new functionality
- **Question** — For usage questions

## Development Workflow

### Branches
- `main` — Stable releases only
- `develop` — Integration branch for next release
- `feat/*` — Feature branches
- `fix/*` — Bug fix branches

### Versioning
- **Semantic Versioning** (MAJOR.MINOR.PATCH)
- ABI stability within MAJOR version
- Vocabulary changes = MINOR version bump (with deprecation period)

## Good First Issues

Look for labels:
- `good first issue` — Small, self-contained tasks
- `help wanted` — Larger tasks needing contributors
- `documentation` — Docs improvements

Examples:
- Add alpha blending, more topologies, or additional software rasterizer tests
- Implement visible-window drawing and complete the Vulkan renderer path
- Add text/fonts, skeletal animation, or post-process effects
- Expand matrix and quaternion helper coverage

## Questions?

Open a [Discussion](https://github.com/<your-org>/helix-rnd/discussions) or ask in a PR.
