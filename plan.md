# Hunnu Language — 6-Month Development Plan

> May 2026 – October 2026
> A focused roadmap from interpreted language to native binary compiler.

---

## Vision

Hunnu is a lightweight, bilingual (English/Mongolian) programming language with an ambitious long-term goal:

1. **Systems programming** — write a Linux-like kernel
2. **Scientific computing** — numpy-like numerical packages
3. **Machine learning** — native ML framework with GPU support

The path: C interpreter → Rust VM → AOT compiler → Kernel + ML ecosystem.

---

## Features

### Working Features

| Feature | English | Mongolian |
|---------|---------|-----------|
| Variables | `let x = 5` | `хувьсагч x = 5` |
| Functions | `fn add(a, b) { return a + b }` | `функц нэмэх(a, b) { буцаах a + b }` |
| If/else | `if x > 0 { ... } else { ... }` | `хэрвээ x > 0 { ... } бусад { ... }` |
| While loop | `while i < 10 { ... }` | `давталт i < 10 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` | `тооллого хувьсагч i = 0; i < 3; i = i + i + 1 { ... }` |
| Print | `print("Hello")` | `хэвлэх("Сайн уу")` |
| Return | `return value` | `буцаах утга` |
| Break | `break` | `зогсоох` |
| Continue | `continue` | `үргэлжлүүлэх` |
| null/nil | `null`, `nil` | `хоосон` |
| Arrays | `let arr = [1, 2, 3]` | `жагсаалт arr = [1, 2, 3]` |
| Index assignment | `arr[0] = 5` | `arr[0] = 5` |
| Imports | `import "lib.hn"` | `импорт "lib.hn"` |
| FFI | `extern fn puts(s: str) -> int from "libc.so"` | `гаднах fn puts(s: str) -> int from "libc.so"` |

---

## CLI Usage

```bash
./hunnu run examples/main.hn          # Interpreter
./hunnu run examples/main.hn --vm     # C VM
./hunnu run examples/main.hn --debug  # Show tokens + AST
./hunnu build examples/main.hn        # Show bytecode
./hunnu tokens examples/main.hn       # Lexer debug
./hunnu ast examples/main.hn          # Parser debug
```

---

## 6-Month Timeline

```
Month 1 (May 2026)    Month 2 (Jun 2026)    Month 3 (Jul 2026)
┌───────────────┐     ┌───────────────┐     ┌───────────────┐
│  Rust VM      │     │  FFI Ecosystem │     │  AOT Compiler │
│  GC / Memory  │  →  │  Standard Lib  │  →  │  Structs      │
│  User Fn Calls│     │  Python Bind   │     │  Pointers     │
│  ──────────── │     │  ────────────  │     │  ──────────── │
│  Goal: Stable │     │  Goal: Usable  │     │  Goal: Native │
│  Rust Runtime │     │  Language      │     │  Binaries     │
└───────────────┘     └───────────────┘     └───────────────┘

Month 4 (Aug 2026)    Month 5 (Sep 2026)    Month 6 (Oct 2026)
┌───────────────┐     ┌───────────────┐     ┌───────────────┐
│  Generics     │     │  no_std / Bare │     │  Self-Hosting │
│  Traits       │  →  │  Metal Target  │  →  │  Package Mgr  │
│  Modules      │     │  Boot Example  │     │  0.1 Release  │
│  ──────────── │     │  ────────────  │     │  ──────────── │
│  Goal: Type   │     │  Goal: Kernel  │     │  Goal: v0.1  │
│  System       │     │  Prototype     │     │  Usable Lang  │
└───────────────┘     └───────────────┘     └───────────────┘
```

---

## Completed Phases

### Phase 1: Foundation Fixes ✅

| # | Feature | Description |
|---|---------|-------------|
| 1 | Variable scoping | Block-scoped variables with scope stack |
| 2 | break/continue | Loop control flow |
| 3 | Array bounds checking | `arr[i]` IndexError |
| 4 | String memory safety | Dangling pointer fixes |

### Phase 2: Core Language Features ✅

| # | Feature | Description |
|---|---------|-------------|
| 1 | Compound assignment | `x += 1`, `x -= 1`, etc. |
| 2 | `else if` chains | Multiple conditions |
| 3 | Floating-point numbers | `3.14`, `2.0` |
| 4 | `null`/`nil` literal | `let x = null` |

### Phase 3: Standard Library + DX ✅

| # | Feature | Description |
|---|---------|-------------|
| 1 | `input()` | Read from stdin |
| 2 | `to_int()`, `to_float()`, `to_str()` | Type conversions |
| 3 | `--debug` flag | Show tokens and AST |

### Phase 4: Bytecode + VM ✅

| # | Feature | Description |
|---|---------|-------------|
| 1 | Bytecode compiler | AST → bytecode |
| 2 | Virtual Machine (C) | Bytecode execution |
| 3 | `build` command | Output bytecode |
| 4 | `--vm` flag | Run with VM |
| 5 | Import statement | `import "lib.hn"` with recursive loading |
| 6 | Error line numbers | Runtime errors show source line |
| 7 | Array deep copy | Proper copy and free semantics |
| 8 | Index assignment | `arr[0] = 5` and `arr[i] += 1` |

---

## Month 1 (May 2026): Rust VM Stabilization ✅ (Completed)

**Goal:** Replace C VM with Rust VM as primary runtime, add user-defined functions.

| # | Feature | Description | Files | Status |
|---|---------|-------------|-------|------------|
| 1 | Rust VM bytecode wire | Pipe C compiler output → Rust VM | `vm-rust/`, `cli/main.c` | ✅ Done |
| 2 | User-defined functions (VM) | Call frames, OP_CALL, OP_RETURN, OP_DEFINE_FN | `vm-rust/vm.rs` | ✅ Done |
| 3 | VM function calls (C) | VM support for user-defined fn calls | `compiler/vm/vm.c` | ✅ Done |
| 4 | GC: reference counting | Automatic memory management | `vm-rust/` | ✅ Done (Rust ownership) |
| 5 | String concatenation in VM | `OP_ADD` for strings | `vm-rust/vm.rs` | ✅ Done |
| 6 | Rust VM tests | Bytecode round-trip tests | `vm-rust/src/lib.rs` | ✅ Done |
| 7 | C + Rust FFI integration | Call Rust VM from C via FFI | `vm-rust/src/lib.rs`, `cli/main.c` | ✅ Done |
| 8 | Integrated CMake build | Build C and Rust together | `CMakeLists.txt` | ✅ Done |

**Milestone:** `./hunnu run --vm-rust` executes full Hunnu programs with functions. ✅ Achieved

**Test Results:** 8/8 Rust VM tests passing ✅

---

## Month 2 (June 2026): FFI Ecosystem + Standard Library

**Goal:** Make Hunnu practically useful with libraries and Python integration.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | libc.hn standard module | Pre-built C FFI bindings (puts, malloc, strlen, etc.) | `stdlib/libc.hn` | Easy |
| 2 | FFI string returns | `extern fn` returning `str` | `compiler/` | Medium |
| 3 | FFI float arguments | Support float params in extern calls | `compiler/` | Easy |
| 4 | Rust FFI boundary | Call Rust `#[no_mangle]` fns from Hunnu | `vm-rust/ffi.rs` | Medium |
| 5 | Python bindings (PyO3) | `import hunnu` in Python, execute `.hn` | `bindings/python/` | Medium |
| 6 | Error handling | `try`/`catch` or `Result`-style errors | `compiler/` | Hard |
| 7 | Module system | `import std.math`, `import std.io` | `stdlib/`, `cli/` | Medium |

**Milestone:** `import std.io; std.io.println("Hello from stdlib!")`

---

## Month 3 (July 2026): AOT Compiler Foundation

**Goal:** Compile Hunnu to native binaries via LLVM.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | Frontend in Rust | Port lexer + parser to Rust | `compiler-rust/` | Hard |
| 2 | LLVM IR codegen | AST → LLVM IR basic blocks | `compiler-rust/codegen.rs` | Very Hard |
| 3 | `hunnu compile` command | Output ELF binary | `cli/` | Medium |
| 4 | Structs / Records | `type Point = { x: int, y: int }` | `compiler/` | Medium |
| 5 | Field access | `point.x`, `point.y` | `compiler/` | Medium |
| 6 | Pointers | `let p = &x`, `*p` | `compiler/` | Hard |

**Milestone:** `hunnu compile main.hn -o main && ./main` prints "Hello".

---

## Month 4 (August 2026): Advanced Type System

**Goal:** Generics, traits, and modules for expressive code.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | Enums / ADTs | `type Result[T] = Ok(T) | Err(String)` | `compiler/` | Hard |
| 2 | Pattern matching | `match x { Ok(v) => ..., Err(e) => ... }` | `compiler/` | Hard |
| 3 | Generics | `fn map[T, U](arr: [T], f: fn(T) -> U) -> [U]` | `compiler/` | Very Hard |
| 4 | Traits / Interfaces | `trait Eq { fn eq(self, other: Self) -> bool }` | `compiler/` | Very Hard |
| 5 | `unsafe` blocks | Explicit unsafe code regions | `compiler/` | Medium |
| 6 | Module system | Public/private visibility, `mod` keyword | `compiler/` | Medium |

**Milestone:** `fn map[T](arr: [T]) -> [T] { ... }` with trait bounds.

---

## Month 5 (September 2026): no_std + Bare Metal

**Goal:** Run Hunnu without OS — boot loader and kernel prototype.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | `no_std` mode | Compile without libc | `compiler-rust/` | Hard |
| 2 | Bare-metal target | x86_64-unknown-none | `compiler-rust/` | Hard |
| 3 | Boot loader | Multiboot2 / UEFI entry point | `kernel/boot.rs` | Very Hard |
| 4 | Memory manager | Physical page allocator | `kernel/alloc.rs` | Very Hard |
| 5 | VGA text buffer | `print!` to screen | `kernel/vga.rs` | Medium |
| 6 | Interrupt handling | IDT, PIC, keyboard IRQ | `kernel/interrupts.rs` | Very Hard |
| 7 | Minimal kernel | Boot, print "Hunnu kernel booted", halt | `kernel/main.rs` | Very Hard |

**Milestone:** QEMU boots Hunnu kernel, prints "Hello from Hunnu!".

---

## Month 6 (October 2026): Self-Hosting + Release

**Goal:** Hunnu compiler written in Hunnu, package manager, v0.1 release.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | Package manager | `hunnu install`, `hunnu new`, dependency resolution | `cli/pkg.rs` | Hard |
| 2 | Standard library v1 | `std.io`, `std.math`, `std.array`, `std.string` | `stdlib/` | Medium |
| 3 | Self-hosting attempt | Write Hunnu lexer in Hunnu | `self/lexer.hn` | Very Hard |
| 4 | Documentation | Language spec, tutorial, API docs | `docs/` | Medium |
| 5 | CI/CD | GitHub Actions: build, test, lint | `.github/workflows/` | Easy |
| 6 | Benchmark suite | Performance comparison with Python, Lua | `benchmarks/` | Medium |
| 7 | v0.1 release | First tagged release on GitHub | — | Medium |

**Milestone:** `hunnu new my-project && hunnu run` produces working app.

---

## Architecture Evolution

```
Month 1-2               Month 3-4               Month 5-6
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│  .hn source  │        │  .hn source  │        │  .hn source  │
└──────┬───────┘        └──────┬───────┘        └──────┬───────┘
       │                       │                       │
┌──────▼───────┐        ┌──────▼───────┐        ┌──────▼───────┐
│  Rust Lexer  │        │  Rust Lexer  │        │  Rust Lexer  │
│  Rust Parser │        │  Rust Parser │        │  Rust Parser │
│  Rust VM     │   →    │  LLVM Codegen│   →    │  LLVM Codegen│
│  FFI/dlopen  │        │  Structs     │        │  no_std      │
│  Python/PyO3 │        │  Generics    │        │  ┌────────┐  │
│              │        │  Traits      │        │  │Kernel  │  │
│              │        │  .elf binary │        │  │Pkg Mgr │  │
│              │        │              │        │  │v0.1    │  │
│              │        │              │        │  └────────┘  │
└──────────────┘        └──────────────┘        └──────────────┘
```

---

## Technical Debt (Current)

| Issue | Location | Severity | Fix Plan |
|-------|----------|----------|----------|
| No garbage collection | interpreter.c | High | Month 1: ref counting in Rust VM |
| VM user-defined functions | vm.c, vm/compiler.c | High | Month 1: call frames |
| Global-only variables | vm/compiler.c | Medium | Month 1: scope stack in VM |
| Identifier resolution | vm/compiler.c | Medium | Month 1: symbol table |
| Memory leaks in VM | vm.c | Medium | Month 1: Rust ownership model |
| No type checking | parser.c | Low | Month 2: type checker pass |
| Hardcoded builtin names | vm.c | Low | Month 2: builtin registry |

---

## Language Design Goals

### For Systems Programming
- Predictable performance (no hidden allocations)
- `unsafe` blocks for low-level operations
- Manual memory management option (no GC)
- Inline assembly support
- No runtime dependency for kernel mode

### For Scientific Computing
- First-class tensor/n-dimensional array type
- Operator overloading (`a @ b` for matrix multiply)
- FFI to BLAS/LAPACK/CuBLAS
- SIMD vectorization
- Complex number support

### For Machine Learning
- Automatic differentiation (`grad(f)(x)`)
- GPU compute shaders
- Neural network primitives
- Training loop abstractions
- Model serialization

---

## Technology Choices

| Component | Current | Month 3+ | Rationale |
|-----------|---------|----------|-----------|
| Lexer/Parser | C | Rust | Safer, better error handling |
| Interpreter | C (tree-walk) | Deprecated | VM is faster |
| VM | C + Rust prototype | Rust | Ownership model eliminates leaks |
| Compiler | — | LLVM + Rust | Native binary output |
| Kernel | — | Rust | no_std, safe systems code |
| Python Bindings | — | PyO3 | Easy embedding |
| Package Manager | — | Rust + Git | Cargo-style deps |

---

## File Structure

### Current
```
hunnu-lang/
├── compiler/
│   ├── lexer/          # Tokenizer (C)
│   ├── parser/        # Parser (C)
│   ├── ast/          # AST nodes (C)
│   ├── interpreter/  # Tree-walk interpreter (C)
│   └── vm/          # Bytecode compiler + VM (C)
├── vm-rust/           # Rust VM [Month 1 focus]
├── cli/              # Command-line interface
├── examples/         # Example programs
├── plan.md           # This file
└── CMakeLists.txt
```

### Target (Month 6)
```
hunnu-lang/
├── compiler-rust/     # Full Rust frontend
│   ├── lexer/
│   ├── parser/
│   ├── ast/
│   ├── typecheck/
│   └── codegen/      # LLVM IR generation
├── vm-rust/          # Rust VM (interpreted mode)
├── kernel/           # Bare-metal kernel
│   ├── boot.rs
│   ├── alloc.rs
│   ├── vga.rs
│   └── main.rs
├── stdlib/           # Standard library
│   ├── core.hn       # Primitives
│   ├── io.hn         # I/O
│   ├── math.hn       # Math
│   └── array.hn      # Array utilities
├── bindings/
│   └── python/       # PyO3 bindings
├── cli/              # Unified CLI
├── examples/
├── benchmarks/
├── docs/
├── Cargo.toml
└── CMakeLists.txt    # Kept for legacy C interpreter
```

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| LLVM codegen complexity | Very High | Start simple: only int/float/strings, expand later |
| Generics implementation | High | Monomorphize at compile time (like Rust), no runtime cost |
| Bare-metal boot | Very High | Use `bootloader` crate, focus on VGA text first |
| Self-hosting timeline | High | Defer to post-v0.1 if not ready |
| Scope creep | Medium | Strict monthly milestones, no feature additions mid-month |

---

## Success Metrics

| Month | Metric | Target |
|-------|--------|--------|
| 1 | Rust VM passes existing .hn tests | 100% |
| 2 | Standard library modules available | 4+ modules |
| 3 | Native binary from `hunnu compile` | "Hello World" |
| 4 | Generic function compiles | `fn id[T](x: T) -> T` |
| 5 | QEMU boots Hunnu kernel | VGA text output |
| 6 | `hunnu new` creates runnable project | Full cycle |

---

## Links

- Web: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang

MIT License © 2025-2026 Hunnu
