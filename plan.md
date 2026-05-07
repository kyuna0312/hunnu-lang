# Hunnu Language v1.0 — 6-Month Development Plan

> May 2026 – October 2026

**Current: v1.0.0 (Эрдэнэ — Jewel)**

## CLI

```bash
hunnu run file.hn                  # C tree-walk interpreter
hunnu run file.hn --vm             # C bytecode VM
hunnu run file.hn --vm-rust        # Rust VM via FFI
hunnu run file.hn --debug          # Show tokens + AST
hunnu build file.hn                # Show bytecode
hunnu compile main.hn -o main      # AOT compile (LLVM in progress)
hunnu tokens file.hn               # Lexer debug
hunnu ast file.hn                  # Parser debug
```

## 6-Month Timeline

```
Month 1 (May 2026)    Month 2 (Jun 2026)    Month 3 (Jul 2026)
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ Rust VM      │     │ FFI Ecosystem │     │ AOT Compiler │
│ GC / Memory  │  →  │ Standard Lib  │  →  │ Structs      │
│ User Fn Calls │     │ Python Bind   │     │ Pointers     │
│ ──────────── │     │ ────────────  │     │ ──────────── │
│ Stable Rust  │     │ Usable Lang   │     │ Native Bins  │
└──────────────┘     └──────────────┘     └──────────────┘

Month 4 (Aug 2026)    Month 5 (Sep 2026)    Month 6 (Oct 2026)
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ Generics     │     │ no_std / Bare │     │ Self-Hosting │
│ Traits       │  →  │ Metal Target  │  →  │ Package Mgr  │
│ FP / OOP     │     │ Boot Example  │     │ v1.0 Release │
│ ──────────── │     │ ────────────  │     │ ──────────── │
│ Type System  │     │ Kernel Proto  │     │ Usable Lang  │
└──────────────┘     └──────────────┘     └──────────────┘
```

## Completed

### Phase 1-4: Foundation + Core Language
- Block-scoped variables, break/continue, array bounds checking
- Compound assignment (`x += 1`), else if, floats, null/nil
- `input()`, `to_int/to_float/to_str`, `--debug` flag
- Bytecode compiler + C VM, `build` command, `--vm` flag
- Import system (quoted paths + module paths like `import std.math`)
- Error line numbers, array deep copy, index assignment

### Month 1: Rust VM
- Rust VM bytecode execution (OP_CALL, OP_RETURN, OP_DEFINE_FN)
- User-defined function calls in both C and Rust VMs
- String concatenation in VM
- C + Rust FFI integration with CMake build
- 8/8 Rust VM tests passing

### Month 2: FFI + Standard Library
- `stdlib/libc.hn` with C FFI bindings (puts, printf, strlen, etc.)
- FFI string returns and float arguments
- Python bindings (PyO3)
- Try/catch error handling
- Module system (`import std.math`, `std.io`, etc.)
- Stdlib modules: libc, math, io, array, string, fs, time

### Month 3: AOT Compiler Foundation
- Rust compiler frontend (lexer, parser, AST)
- LLVM IR codegen skeleton
- `hunnu compile` command with C fallback
- Struct/record types (`type Point = { x, y }`)
- Field access (`point.x`), pointers (`&x`, `*p`)
- Address-of and dereference operators

## Month 3 (July 2026): AOT Compiler Foundation

| # | Feature | Files | Status |
|--|---------|-------|--------|
| 1 | Rust frontend (lexer + parser) | `compiler-rust/` | ✅ Done |
| 2 | LLVM IR codegen | `compiler-rust/codegen.rs` | ✅ Feature-gated (`llvm-codegen`) |
| 3 | `hunnu compile` | `cli/main.c` | ✅ Done (gcc fallback) |
| 4 | Structs/records | `compiler/`, `compiler-rust/` | ✅ Done |
| 5 | Field access | `compiler/` | ✅ Done |
| 6 | Pointers | `compiler/` | ✅ Done |

**Prerequisite:** LLVM dev headers (`sudo pacman -S llvm` on Arch, `sudo apt install llvm-dev` on Debian).
Build with LLVM: `cd compiler-rust && cargo build --features llvm-codegen`.

**Milestone:** `hunnu compile main.hn -o main && ./main` prints "Hello".

## Month 4 (August 2026): Type System + FP/OOP

**Type system:**

| # | Feature | Difficulty |
|---|---------|------------|
| 1 | Enums / ADTs | Hard |
| 2 | Pattern matching | Hard |
| 3 | Generics | Very Hard |
| 4 | Traits / Interfaces | Very Hard |
| 5 | `unsafe` blocks | Medium |
| 6 | Module system (pub/priv) | Medium |

**Functional programming:**

| # | Feature | Difficulty |
|---|---------|------------|
| 1 | Immutable by default, `mut` keyword | Medium |
| 2 | First-class functions, closures | Hard |
| 3 | Higher-order functions (map, filter, reduce) | Medium |
| 4 | Lambda expressions (`\|x\| x + 1`) | Hard |
| 5 | Pattern matching with destructuring | Hard |
| 6 | Tail call optimization | Very Hard |

**Object-oriented programming:**

| # | Feature | Difficulty |
|---|---------|------------|
| 1 | Structs with methods | Medium |
| 2 | Classes with `new` | Very Hard |
| 3 | Inheritance | Very Hard |
| 4 | Polymorphism (vtable) | Very Hard |
| 5 | Encapsulation (public/private) | Medium |
| 6 | Interfaces/Traits | Very Hard |
| 7 | `this`/`self` reference | Medium |

**Milestone:** Generic functions, FP (map/filter/reduce), OOP classes.

## Month 5 (September 2026): no_std + Bare Metal

| # | Feature | Difficulty |
|---|---------|------------|
| 1 | `no_std` mode (compile without libc) | Hard |
| 2 | Bare-metal target (x86_64-unknown-none) | Hard |
| 3 | Boot loader (Multiboot2 / UEFI) | Very Hard |
| 4 | Physical page allocator | Very Hard |
| 5 | VGA text buffer driver | Medium |
| 6 | Interrupt handling (IDT, PIC, keyboard) | Very Hard |
| 7 | Minimal kernel (boot → print → halt) | Very Hard |

**Milestone:** QEMU boots Hunnu kernel, prints "Hello from Hunnu!".

## Month 6 (October 2026): Self-Hosting + Release

| # | Feature | Difficulty |
|---|---------|------------|
| 1 | Package manager (`hunnu install`, `hunnu new`) | Hard |
| 2 | Standard library v1 | Medium |
| 3 | Self-hosting (Hunnu lexer in Hunnu) | Very Hard |
| 4 | Documentation + language spec | Medium |
| 5 | CI/CD (GitHub Actions) | Easy |
| 6 | Benchmark suite | Medium |
| 7 | v1.0 release | Medium |

**Milestone:** `hunnu new my-project && hunnu run` produces a working app.

## Architecture Evolution

```
Month 1-2               Month 3-4               Month 5-6
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│ .hn source   │        │ .hn source   │        │ .hn source   │
└──────┬───────┘        └──────┬───────┘        └──────┬───────┘
       │                       │                       │
┌──────▼───────┐        ┌──────▼───────┐        ┌──────▼───────┐
│ Rust Lexer   │        │ Rust Lexer   │        │ Rust Lexer   │
│ Rust Parser  │        │ Rust Parser  │        │ Rust Parser  │
│ Rust VM      │   →    │ LLVM Codegen │   →    │ LLVM Codegen │
│ FFI/dlopen   │        │ Structs      │        │ no_std       │
│ Python/PyO3  │        │ Generics     │        │ ┌────────┐  │
│              │        │ Traits       │        │ │Kernel  │  │
│              │        │ FP / OOP     │        │ │Pkg Mgr │  │
│              │        │ .elf binary  │        │ │v1.0   │  │
└──────────────┘        └──────────────┘        └──────────────┘
```

## Project Structure

```
hunnu-lang/
├── compiler/
│   ├── ast/           # AST nodes (create/free/print)
│   ├── interpreter/   # C tree-walk interpreter
│   ├── lexer/         # Tokenizer
│   ├── parser/        # Recursive descent parser
│   ├── vm/            # Bytecode compiler + C VM
│   ├── i18n/          # English/Mongolian support
│   ├── value.c/h      # Core Value type (shared by interpreter + VM)
│   ├── scope.c/h      # Variable scope management
│   └── import.c/h     # Import resolution (stdlib paths, env var)
├── compiler-rust/     # Rust AOT compiler frontend (LLVM)
├── vm-rust/           # Rust bytecode VM (staticlib + binary)
├── cli/               # CLI entry point + command dispatch
├── stdlib/            # Standard library modules
│   ├── libc.hn       # C FFI bindings
│   ├── math.hn, io.hn, array.hn
│   ├── string.hn, fs.hn, time.hn
├── bindings/python/   # PyO3 Python bindings
├── examples/          # Sample .hn programs
├── tasks.md           # Task tracker with feature status
├── install.sh         # Linux/macOS installer
├── install.bat        # Windows installer
└── CMakeLists.txt     # C + Rust build config
```

## Language Design Goals

### Systems Programming
- Predictable performance (no hidden allocations)
- `unsafe` blocks for low-level operations
- Manual memory management option
- Inline assembly, no runtime for kernel mode

### Scientific Computing
- First-class tensor/n-dimensional array type
- Operator overloading
- FFI to BLAS/LAPACK/CuBLAS
- SIMD vectorization, complex numbers

### Machine Learning
- Automatic differentiation (`grad(f)(x)`)
- GPU compute shaders
- Neural network primitives, model serialization

### Multi-Paradigm (FP + OOP)
- Immutable by default, first-class functions, closures
- Higher-order functions (map, filter, reduce)
- Pattern matching with destructuring, TCO
- Classes with inheritance, polymorphism, encapsulation
- Interfaces/traits, mix and match paradigms

## Technology Choices

| Component | Current | Month 3+ | Rationale | Build |
|-----------|---------|----------|-----------|-------|
| Lexer/Parser | C | Rust | Safer, better errors | `default` |
| Interpreter | C (tree-walk) | Deprecated | VM is faster | `default` |
| VM | C + Rust (FFI) | Rust | Ownership eliminates leaks | `default` |
| Compiler | C (bytecode) | LLVM + Rust | Native binary output | `--features llvm-codegen` |
| Kernel | — | Rust | no_std, safe systems code | future |
| Python Bindings | — | PyO3 | Easy embedding | future |
| Package Manager | — | Rust + Git | Cargo-style deps | future |

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| LLVM codegen complexity | Very High | Start simple: int/float/strings only |
| Generics implementation | High | Monomorphize at compile time (like Rust) |
| Bare-metal boot | Very High | Use `bootloader` crate, VGA text first |
| Self-hosting timeline | High | Defer to post-v1.0 if not ready |
| Scope creep | Medium | Strict monthly milestones |
| FP/OOP complexity | High | Incremental: structs → classes → traits |

## Success Metrics

| Month | Metric | Target |
|-------|--------|--------|
| 1 | Rust VM passes .hn tests | 100% |
| 2 | Stdlib modules available | 4+ modules |
| 3 | `hunnu compile` produces native binary | "Hello World" |
| 4 | Generic function compiles | `fn id[T](x: T) -> T` |
| 5 | QEMU boots Hunnu kernel | VGA text output |
| 6 | `hunnu new` creates runnable project | Full cycle + release |

## Technical Debt

| Issue | Location | Status |
|-------|----------|--------|
| No GC | interpreter.c | ✅ Fixed (Rust ownership) |
| VM user-defined functions | vm.c | ✅ Fixed (call frames) |
| Global-only variables | vm/compiler.c | ✅ Fixed (scope stack) |
| Memory leaks in VM | vm.c | ✅ Fixed (Rust) |
| No type checking | parser.c | Pending |
| Hardcoded builtin names | vm.c | Pending |
| C + Rust FFI integration | cli/main.c, vm-rust/ | ✅ Fixed |

## Links

- Web: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang

MIT License © 2025-2026 Hunnu
