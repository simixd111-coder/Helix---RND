# Security Policy

## Supported Versions

We release security updates for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |

## Reporting a Vulnerability

**Please do NOT report security vulnerabilities via public GitHub issues.**

Instead, please report them via email to **security@helix-rnd.org**.

You should receive a response within 48 hours. If for some reason you do not,
please follow up via email to ensure we received your original message.

Please include the following information in your report:

- Type of issue (e.g., buffer overflow, use-after-free, integer overflow, etc.)
- Full paths of source file(s) related to the issue
- The location of the affected source code (tag/branch/commit or URL)
- Any special configuration required to reproduce the issue
- Step-by-step instructions to reproduce the issue
- Proof-of-concept or exploit code (if possible)
- Impact of the issue, including how an attacker might exploit it

## Disclosure Policy

- We will acknowledge receipt of your vulnerability report within 48 hours
- We will provide a more detailed response within 7 days indicating next steps
- We will keep you informed of our progress throughout the process
- We will credit you in the security advisory (unless you prefer to remain anonymous)
- We will coordinate the release of the security advisory and patch with you

## Security Best Practices for Contributors

- All code must compile with `-Wall -Wextra -Werror` (or MSVC equivalent)
- Use sanitizers (ASan, UBSan, MSan) during development
- Run `ctest` before submitting PRs
- Never commit secrets, keys, or credentials
- Dependencies are vendorized — audit third-party code before updating

## Scope

This security policy applies to the Helix RND core engine (`include/helix.h`, `src/`).
Language bindings, packaging scripts, and documentation are out of scope but
should follow the same principles.