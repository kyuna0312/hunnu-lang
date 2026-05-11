# Hunnu Language v1.0 — 6-Month Development Plan

> May 2026 – October 2026

**Current: v1.0.0 (Эрдэнэ — Jewel) — Month 4 done, FP+OOP+Type System complete**

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

Month 4 (Aug 2026)    Month 6 (Oct 2026)    Post-v1.0
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ Type System  │     │ Self-Hosting │     │ Vision Feats │
│ OOP (done)   │  →  │ Package Mgr  │  →  │ String Interp │
│ FP (done)    │     │ v1.0 Release │     │ Keyword Ops  │
│ ──────────── │     │ ──────────── │     │ ──────────── │
│ All Complete │     │ Usable Lang  │     │ Stdlib Parity│
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

| # | Feature | Difficulty | Status |
|---|---------|------------|--------|
| 1 | Enums / ADTs | Hard | ✅ **Done** |
| 2 | Pattern matching | Hard | ✅ **Done** |
| 3 | Generics | Very Hard | ✅ **Done** |
| 4 | Traits / Interfaces | Very Hard | ✅ **Done** |
| 5 | `unsafe` blocks | Medium | ✅ **Done** |
| 6 | Module system (pub/priv) | Medium | ✅ **Done** |

**Functional programming:**

| # | Feature | Difficulty | Status |
|---|---------|------------|--------|
| 1 | Immutable by default, `mut` keyword | Medium | ✅ **Done** |
| 2 | First-class functions, closures | Hard | ✅ **Done** |
| 3 | Higher-order functions (map, filter, reduce) | Medium | ✅ **Done** |
| 4 | Lambda expressions (`\|x\| x + 1`) | Hard | ✅ **Done** |
| 5 | Pattern matching with destructuring | Hard | ✅ **Done** |
| 6 | Tail call optimization | Very Hard | ✅ **Done** |

**Object-oriented programming:**

| # | Feature | Difficulty | Status |
|---|---------|------------|--------|
| 1 | Structs with methods | Medium | ✅ **Done** |
| 2 | Classes with `new` | Very Hard | ✅ **Done** |
| 3 | Inheritance | Very Hard | ✅ **Done** |
| 4 | Polymorphism (vtable) | Very Hard | ✅ **Done** |
| 5 | Encapsulation (public/private) | Medium | ✅ **Done** |
| 6 | Interfaces/Traits | Very Hard | ✅ **Done** |
| 7 | `this`/`self` reference | Medium | ✅ **Done** |

**Milestone:** Type system, OOP, and FP all complete.

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

## Post-v1.0: Vision alignment (from hunnu-opencode-prompt.md)

Features defined in the language vision document (`hunnu-opencode-prompt.md`) that are not yet implemented:

| # | Feature | Difficulty | Sub-issue |
|---|---------|------------|-----------|
| 1 | `def`/`end` block syntax (Ruby-style) | Hard | #54 |
| 2 | String interpolation (`#{}`) | Medium | #55 |
| 3 | Range patterns and array destructuring in match | Medium | #56 |
| 4 | Option/Result types (no null) | Very Hard | #57 |
| 5 | `and`/`or`/`not` keyword operators | Easy | #58 |
| 6 | Stdlib API parity (`upcase`, `include?`, `first`, etc.) | Medium | #59 |
| 7 | Symbol type (`:ok`, `:err`) | Medium | #60 |

**Design principles from prompt:**
- Both keywords always work (EN + MN for every keyword)
- Ruby feel, compiled performance
- No null — use `Option[T]`
- Errors are values — `Result[T, E]`
- Mongolian is not an afterthought
- SOV-friendly syntax (trailing `if`/`unless`, method chaining)
- No semicolons, implicit return
- Clean bilingual compiler errors
- One binary output with no runtime dependency

## Architecture Evolution

```
Month 1-2               Month 3-4               Month 6+
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│ .hn source   │        │ .hn source   │        │ .hn source   │
└──────┬───────┘        └──────┬───────┘        └──────┬───────┘
       │                       │                       │
┌──────▼───────┐        ┌──────▼───────┐        ┌──────▼───────┐
│ Rust Lexer   │        │ Rust Lexer   │        │ Rust Lexer   │
│ Rust Parser  │        │ Rust Parser  │        │ Rust Parser  │
│ Rust VM      │   →    │ LLVM Codegen │   →    │ LLVM Codegen │
│ FFI/dlopen   │        │ Type System  │        │ Vision Feats │
│ Python/PyO3  │        │ FP / OOP     │        │ Self-Hosting │
│              │        │ .elf binary  │        │ Pkg Mgr/v1.0 │
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

| Python Bindings | — | PyO3 | Easy embedding | future |
| Package Manager | — | Rust + Git | Cargo-style deps | future |

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| LLVM codegen complexity | Very High | Start simple: int/float/strings only |
| Generics implementation | High | Monomorphize at compile time (like Rust) |
| Self-hosting timeline | High | Defer to post-v1.0 if not ready |
| Scope creep | Medium | Strict monthly milestones |
| FP/OOP complexity | High | Incremental: structs → classes → traits |

## Success Metrics

| Month | Metric | Target | Status |
|-------|--------|--------|--------|
| 1 | Rust VM passes .hn tests | 100% | ✅ |
| 2 | Stdlib modules available | 4+ modules | ✅ |
| 3 | `hunnu compile` produces native binary | "Hello World" | ✅ |
| 4 | OOP classes, inheritance, traits working | 26/26 tests pass | ✅ |
| 4 | Type system: enums, generics, unsafe, pub/priv | Complete | ✅ |
| 4 | FP: mut, lambdas, first-class fns, HOFs, TCO | map/filter/reduce work | ✅ |
| 6 | `hunnu new` creates runnable project | Full cycle + release | Pending |

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
