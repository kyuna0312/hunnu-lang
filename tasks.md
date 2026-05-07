# Hunnu Language Task Tracker

> Generated: May 2026 | Current branch: `refactor/extract-value-scope-import`

## How to Use

```
[X] = Complete
[/] = In progress
[ ] = Not started
[~] = Blocked/deferred
```

---

## Month 3 (Jul 2026): AOT Compiler Foundation

**Goal:** `hunnu compile main.hn -o main && ./main` prints "Hello".

### Rust Frontend

| # | Task | Status | Notes |
|---|------|--------|-------|
| 1 | Rust lexer | [X] | `compiler-rust/src/lexer.rs` |
| 2 | Rust parser | [X] | `compiler-rust/src/parser.rs` |
| 3 | Wire Rust frontend as alternative lex/parse path | [ ] | CLI flag `--rust-lex` or auto-detect |
| 4 | Rust compiler tests | [ ] | Unit tests for lexer + parser |

### LLVM Codegen

| # | Task | Status | Notes |
|---|------|--------|-------|
| 1 | LLVM install + llvm-sys setup | [ ] | Needs `sudo pacman -S llvm` or equivalent |
| 2 | Feature-gate codegen module | [ ] | Behind `llvm-codegen` feature in Cargo.toml |
| 3 | Fix `generate_var_decl` (alloca + store) | [ ] | Currently broken (uses SetInitializer) |
| 4 | Fix `generate_fn_decl` (params, return) | [ ] | Missing param handling |
| 5 | Fix `generate_print` (printf call builder) | [ ] | Incomplete call construction |
| 6 | Add string literal support | [ ] | Global string constants |
| 7 | Add bool/float codegen paths | [ ] | For Month 4 type system |
| 8 | JIT-compile + run (LLVM ORC) | [ ] | Optional JIT mode |
| 9 | Object file emission | [ ] | `.o` file output + system linker |

### CLI Integration

| # | Task | Status | Notes |
|---|------|--------|-------|
| 1 | `hunnu compile` → Rust LLVM codegen | [ ] | Replace gcc fallback |
| 2 | `hunnu compile --ir` → dump LLVM IR | [ ] | Debug mode |
| 3 | Cross-platform compile (gcc/clang detection) | [ ] | For fallback path |

---

## Month 4 (Aug 2026): Type System + FP/OOP

### Type System

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Enums / ADTs | [ ] | |
| 2 | Pattern matching | [X] | In C interpreter |
| 3 | Generics | [ ] | Monomorphization |
| 4 | Traits / Interfaces | [ ] | |
| 5 | `unsafe` blocks | [ ] | |
| 6 | Module system (pub/priv) | [ ] | |

### FP

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Immutable by default, `mut` keyword | [ ] | |
| 2 | First-class functions, closures | [ ] | |
| 3 | Higher-order functions (map, filter, reduce) | [ ] | |
| 4 | Lambda expressions (`\|x\| x + 1`) | [ ] | |
| 5 | Tail call optimization | [ ] | |

### OOP

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Structs with methods | [ ] | |
| 2 | Classes with `new` | [ ] | |
| 3 | Inheritance | [ ] | |
| 4 | Polymorphism (vtable) | [ ] | |
| 5 | Encapsulation (public/private) | [ ] | |
| 6 | `this`/`self` reference | [ ] | |

---

## Month 5 (Sep 2026): no_std + Bare Metal

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | `no_std` mode | [ ] | |
| 2 | Bare-metal target | [ ] | |
| 3 | Boot loader | [ ] | |
| 4 | Physical page allocator | [ ] | |
| 5 | VGA text buffer driver | [ ] | |
| 6 | Interrupt handling | [ ] | |
| 7 | Minimal kernel | [ ] | |

---

## Month 6 (Oct 2026): Self-Hosting + Release

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Package manager (`hunnu install`, `hunnu new`) | [ ] | |
| 2 | Standard library v1 | [ ] | |
| 3 | Self-hosting (Hunnu lexer in Hunnu) | [ ] | |
| 4 | Documentation + language spec | [ ] | |
| 5 | CI/CD (GitHub Actions) | [ ] | |
| 6 | Benchmark suite | [ ] | |
| 7 | v1.0 release | [ ] | |

---

## Technical Debt / Code Health

| # | Task | Priority | Notes |
|---|------|----------|-------|
| 1 | Split `parser.c` (1135 lines) | Medium | Could split into declaration/statement/expression |
| 2 | Split `interpreter.c` (~700 lines) | Low | |
| 3 | Split `vm/vm.c` (520 lines) | Low | |
| 4 | C unit test framework | Medium | No tests for C code currently |
| 5 | Rust unit tests for lexer/parser | Medium | |
| 6 | Memory leak audit (C interpreter + VM) | Low | |

---

## Build Notes

### LLVM Dependency

The Rust LLVM codegen requires LLVM dev headers. On Arch/Manjaro:

```bash
sudo pacman -S llvm
```

On Debian/Ubuntu:

```bash
sudo apt install llvm-dev
```

Without LLVM, the `compiler-rust` crate builds with only the lexer + parser (codegen is feature-gated).

### Quick Build

```bash
cd build && cmake .. && make -j$(nproc)
```

### Run

```bash
./build/hunnu run examples/main.hn
```
