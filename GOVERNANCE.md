# Governance

## Project Leadership

Helix RND is led by a **Core Team** of maintainers who have demonstrated
sustained, high-quality contributions to the project.

### Core Team Responsibilities

- Review and merge pull requests
- Triage issues and prioritize work
- Make decisions on API design, vocabulary, and architecture
- Manage releases and versioning
- Enforce Code of Conduct

### Current Core Team

- [@your-github-handle](https://github.com/your-github-handle) — Project Founder

## Decision Making

### Vocabulary & ABI Changes

Changes to the public C API (`helix.h`) and vocabulary (`docs/VOCABULARY.md`)
require **consensus of the Core Team**.

Process:
1. Open a GitHub Discussion or Issue proposing the change
2. Core Team discusses for at least 7 days
3. Decision recorded in `docs/DECISIONS.md` with rationale
4. If approved, implemented in a single PR with version bump

### Other Changes

- Bug fixes, internal refactors, documentation: Single Core Team member approval
- New features: At least 2 Core Team members approval
- Breaking changes (MAJOR version): Full Core Team consensus

## Adding Core Team Members

A contributor may be invited to join the Core Team when they have:
- Contributed significantly over at least 6 months
- Demonstrated deep understanding of the codebase and vocabulary
- Shown good judgment in reviews and discussions
- Been nominated by an existing Core Team member
- Approved by consensus of current Core Team

## Removing Core Team Members

A Core Team member may be removed for:
- Extended inactivity (12+ months without contribution)
- Violation of Code of Conduct
- Consensus of remaining Core Team members

## Releases

### Versioning

- **Semantic Versioning** (MAJOR.MINOR.PATCH)
- MAJOR: Breaking ABI/API changes (vocabulary changes)
- MINOR: New features, new tokens (backward compatible)
- PATCH: Bug fixes only

### Release Process

1. Update `CHANGELOG.md` with all changes since last release
2. Bump version in `CMakeLists.txt` and `include/helix.h`
3. Tag release: `git tag -a vX.Y.Z -m "Release vX.Y.Z"`
4. Build and test on all supported platforms
5. Publish packages (when bindings exist)
6. Announce in GitHub Discussions

## Conflict Resolution

Disagreements between Core Team members:
1. Discuss in private (GitHub Discussions or email)
2. If unresolved, seek mediation from a neutral third party
3. Final decision by simple majority vote of Core Team

## License

All contributions are made under the MIT License (see `LICENSE`).
Contributors retain copyright to their contributions.