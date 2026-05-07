# Hunnu Roadmap

The future of Hunnu language — features, fixes, and goals.

---

## Current Status ✅

Working features:
- Variables (`let`)
- Functions (`fn`)
- If/Else
- While loops (`while`)
- For loops (`for`)
- Break/Continue
- Arithmetic (`+`, `-`, `*`, `/`, `%`)
- Comparison (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- Print statements
- Arrays (syntax: `[1, 2, 3]` + indexing)
- String concatenation (`+`)
- Floating-point numbers
- null/nil literals
- Input function
- Type conversions (to_int, to_float, to_str)
- Compound assignment (`+=`, `-=`, etc.)
- else if chains
- String escapes
- Bytecode compiler + VM
- **FFI (Foreign Function Interface)** - call C/Rust functions
- **Try/Catch error handling**
- **Module system** - `import std.math` syntax
- **Standard library** - stdlib/ modules (math, io, array, string, fs, time)

Known bugs:
- None currently

---

## Completed Phases

### Phase 1: Foundation Fixes ✅
- ✅ Variable scoping (scope stack)
- ✅ break/continue (loop control flow)
- ✅ String memory safety (dangling pointer fixes)

### Phase 2: Core Language Features ✅
- ✅ Compound assignment (`x += 1`, etc.)
- ✅ `else if` chains
- ✅ Floating-point numbers
- ✅ `null`/`nil` literal

### Phase 3: Standard Library + DX ✅
- ✅ `input()` function
- ✅ `to_int()`, `to_float()`, `to_str()` conversions
- ✅ `--debug` flag (show tokens and AST)

### Phase 4: Bytecode + VM ✅
- ✅ Bytecode compiler (AST → bytecode)
- ✅ Virtual Machine (C)
- ✅ `build` command (output bytecode)
- ✅ `--vm` flag (run with VM)

### Month 1: Rust VM Stabilization ✅
- ✅ Rust VM bytecode wire (pipe C compiler → Rust VM)
- ✅ User-defined functions in VM
- ✅ GC: reference counting (Rust ownership)
- ✅ String concatenation in VM
- ✅ Rust VM tests (8/8 passing)
- ✅ C + Rust FFI integration

### Month 2: FFI Ecosystem + Standard Library ✅
- ✅ libc.hn standard module (C FFI bindings)
- ✅ FFI string returns (`extern fn` returning `str`)
- ✅ FFI float arguments (float params in extern calls)
- ✅ Rust FFI boundary (call Rust functions from Hunnu)
- ✅ Error handling (try/catch syntax)
- ✅ Module system (import std.math syntax)
- ✅ Standard library expansion (array, string, fs, time modules)
- ✅ Python bindings structure (PyO3)

---

## Month 3: AOT Compiler Foundation (July 2026)

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

## Month 4: Advanced Type System + FP/OOP (August 2026)

**Goal:** Generics, traits, modules, functional programming, and object-oriented programming.

### Type System
- Enums / ADTs
- Pattern matching
- Generics
- Traits / Interfaces
- `unsafe` blocks
- Module system (public/private visibility)

### Functional Programming
- Immutable variables (`let x = 5` default, `mut` for mutable)
- First-class functions
- Higher-order functions (map, filter, reduce)
- Lambda expressions
- Pattern matching with destructuring
- Tail call optimization

### Object-Oriented Programming
- Structs/Records with methods
- Classes with inheritance
- Polymorphism (vtable)
- Encapsulation (public/private)
- Interfaces/Traits
- `this`/`self` reference

---

## Month 5: no_std + Bare Metal (September 2026)

**Goal:** Run Hunnu without OS — boot loader and kernel prototype.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | `no_std` mode | Compile without libc | `compiler-rust/` | Hard |
| 2 | Bare-metal target | x86_64-unknown-none | `compiler-rust/` | Hard |
| 3 | Boot loader | Multiboot2 / UEFI entry point | `kernel/boot.rs` | Very Hard |
| 4 | Memory manager | Physical page allocator | `kernel/alloc.rs` | Very Hard |
| 5 | VGA text buffer | `print!` to screen | `kernel/vga.rs` | Medium |
| 6 | Interrupt handling | IDT, PIC, keyboard IRQ | `kernel/interrupts.rs` | Very Hard |
| 7 | Minimal kernel | Boot, print, halt | `kernel/main.rs` | Very Hard |

**Milestone:** QEMU boots Hunnu kernel, prints "Hello from Hunnu!".

---

## Month 6: Self-Hosting + Release (October 2026)

**Goal:** Hunnu compiler written in Hunnu, package manager, v1.0 release.

| # | Feature | Description | Files | Difficulty |
|---|---------|-------------|-------|------------|
| 1 | Package manager | `hunnu install`, `hunnu new` | `cli/pkg.rs` | Hard |
| 2 | Standard library v1 | `std.io`, `std.math`, etc. | `stdlib/` | Medium |
| 3 | Self-hosting attempt | Write Hunnu lexer in Hunnu | `self/lexer.hn` | Very Hard |
| 4 | Documentation | Language spec, tutorial | `docs/` | Medium |
| 5 | CI/CD | GitHub Actions | `.github/workflows/` | Easy |
| 6 | Benchmark suite | Performance comparison | `benchmarks/` | Medium |
| 7 | v1.0 release | First tagged release | — | Medium |

**Milestone:** `hunnu new my-project && hunnu run` produces working app.

---

## Priority Order

| Priority | Item |
|----------|------|
| ✅ Critical | Phase 1-4 Complete |
| ✅ High | Month 1-2 Complete |
| 🟡 Medium | Month 3 (AOT Compiler) |
| 🟢 Medium | Month 4 (Type System + FP/OOP) |
| 🔵 Stretch | Month 5 (no_std + Kernel) |
| 🔵 Stretch | Month 6 (Self-Hosting + v1.0) |

---

## Contributing:

Ideas welcome! Open an issue or pull request.

---

## See Also:

- [`future-improvements.md`](future-improvements.md) - Detailed technical notes
- [`task.md`](task.md) - Session-by-session tasks
- [`plan.md`](plan.md) - Stream schedule
- [`CHANGELOG.md`](CHANGELOG.md) - Version history