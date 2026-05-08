# Contributing to Hunnu

Thank you for your interest in contributing to Hunnu! This document provides guidelines and instructions for contributing.

## Code of Conduct

By participating, you agree to uphold the [Code of Conduct](CODE_OF_CONDUCT.md).

## How to Contribute

### Reporting Bugs

- Check if the bug has already been reported in [Issues](https://github.com/hunnu-labs/hunnu-lang/issues)
- Use the bug report issue template
- Include: OS version, build output, minimal reproduction, expected vs actual behavior

### Feature Requests

- Check existing issues for similar requests
- Use the feature request issue template
- Explain the use case and motivation clearly

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feat/your-feature`)
3. Follow the code style guidelines (see below)
4. Write or update tests as needed
5. Ensure the build passes: `cd build && cmake .. && make`
6. Run linting/typecheck if applicable
7. Submit a pull request against the `main` branch

## Development Setup

```bash
# Clone the repository
git clone https://github.com/hunnu-labs/hunnu-lang.git
cd hunnu-lang

# Build with CMake
mkdir -p build && cd build
cmake ..
make

# Run a program
./hunnu run ../examples/main.hn
```

### LLVM Codegen (optional)

```bash
# Install LLVM dev headers
# Arch: sudo pacman -S llvm
# Debian: sudo apt install llvm-dev

# Build Rust compiler with LLVM
cd compiler-rust && cargo build --features llvm-codegen
```

## Code Style

See [AGENTS.md](AGENTS.md) for detailed code style guidelines. Key points:

- **Formatting:** 4-space indentation, 100-char line max, opening braces on same line
- **Naming:** `snake_case` for variables/functions/files, `UPPER_SNAKE_CASE` for constants/enums/macros
- **Types:** Fixed-width integers from `<stdint.h>`, `size_t` for sizes/indices
- **Error handling:** Return error codes, use `fprintf(stderr, ...)`, clean up resources on error
- **Comments:** Comment "why", not "what". No Chinese characters in code, comments, or docs.

## Project Structure

```
compiler/         # C interpreter (lexer, parser, AST, VM)
compiler-rust/    # Rust AOT compiler frontend (LLVM)
vm-rust/          # Rust bytecode VM
cli/              # CLI entry point
stdlib/           # Standard library (.hn modules)
bindings/python/  # Python bindings (PyO3)
examples/         # Sample programs
```

## Testing

```bash
# Build and run examples
cd build && make
./hunnu examples/main.hn

# Rust tests
cd compiler-rust && cargo test
cd vm-rust && cargo test
```

## Commit Guidelines

- Write clear, concise commit messages describing the "why" not the "what"
- Use the imperative mood ("Add feature" not "Added feature")
- Reference issues and pull requests where applicable

## Questions?

Open a [Discussion](https://github.com/hunnu-labs/hunnu-lang/discussions) or reach out.
