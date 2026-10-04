# Pull Request Template

## Description
Brief description of what this PR does.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactor (no functional change)
- [ ] Test addition
- [ ] Build/CI change

## Vocabulary Changes
If this PR adds/modifies/removes tokens in `helix.h` and `docs/VOCABULARY.md`:

| Token | Old Name | New Name | Action |
|-------|----------|----------|--------|
| `hx_xxx` | | | Added/Modified/Removed |

## Testing
- [ ] All existing tests pass (`ctest --output-on-failure`)
- [ ] New tests added for new functionality
- [ ] Tested on: [Windows / Linux / macOS]
- [ ] Tested with backend: [Software / Vulkan / OpenGL / Metal]

## Code Quality
- [ ] Code formatted with `clang-format`
- [ ] No new compiler warnings
- [ ] SPDX license header on new files (`// SPDX-License-Identifier: MIT`)
- [ ] Follows naming convention (`hx_<verb>_<object>`)

## Documentation
- [ ] `docs/VOCABULARY.md` updated
- [ ] `README.md` updated (if user-facing)
- [ ] `CHANGELOG.md` updated (under [Unreleased])
- [ ] Code comments added for complex logic

## Checklist
- [ ] My code follows the project's style guidelines
- [ ] I have performed a self-review of my code
- [ ] I have commented my code where necessary
- [ ] I have made corresponding changes to documentation
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix/feature works
- [ ] New and existing unit tests pass locally

## Screenshots / Demo Output
If applicable, add screenshots or demo output (e.g., triangle.png from headless demo).

## Related Issues
Closes #<issue-number>