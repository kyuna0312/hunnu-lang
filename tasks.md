# Hunnu Language Task Tracker

> Generated: May 2026 | Current: Month 4 Type System complete (unsafe, enums, generics, pub/priv)

## How to Use

```
[X] = Complete
[/] = In progress
[ ] = Not started
[~] = Blocked/deferred
```

---

## Month 3 (Jul 2026): AOT Compiler Foundation (done)

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
| 1 | Enums / ADTs | [X] | Declared with `enum`, constructed with `Enum::Variant(args)`, matchable |
| 2 | Pattern matching | [X] | In C interpreter |
| 3 | Generics | [X] | `<T>` syntax on functions, type-erased at runtime |
| 4 | Traits / Interfaces | [X] | trait/impl syntax works in C interpreter |
| 5 | `unsafe` blocks | [X] | `unsafe { ... }` block executes body |
| 6 | Module system (pub/priv) | [X] | `pub` on top-level declarations tracked |

### FP

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Immutable by default, `mut` keyword | [ ] | Not started (#31) |
| 2 | First-class functions, closures | [ ] | Not started (#32) |
| 3 | Higher-order functions (map, filter, reduce) | [ ] | Stubs exist but non-functional (#34) |
| 4 | Lambda expressions (`\|x\| x + 1`) | [ ] | Not started (#33) |
| 5 | Tail call optimization | [ ] | Not started (#35) |

### OOP (all done)

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Structs with methods | [X] | Type.method syntax |
| 2 | Classes with `new` | [X] | class/fn new syntax |
| 3 | Inheritance | [X] | class Child : Parent |
| 4 | Polymorphism (vtable) | [X] | Inheritance chain dispatch |
| 5 | Encapsulation (public/private) | [X] | pub keyword enforcement |
| 6 | `this`/`self` reference | [X] | self keyword |

---

## Month 5 (Sep 2026): no_std + Bare Metal

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | `no_std` mode | [ ] | Not started (#36) |
| 2 | Bare-metal target | [ ] | Not started (#37) |
| 3 | Boot loader | [ ] | Not started (#37) |
| 4 | Physical page allocator | [ ] | Not started (#38) |
| 5 | VGA text buffer driver | [ ] | Not started (#39) |
| 6 | Interrupt handling | [ ] | Not started (#39) |
| 7 | Minimal kernel | [ ] | Not started (#40) |

---

## Month 6 (Oct 2026): Self-Hosting + Release

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | Package manager (`hunnu install`, `hunnu new`) | [ ] | Not started (#41) |
| 2 | Standard library v1 | [/] | Partial (4/7 modules, #42) |
| 3 | Self-hosting (Hunnu lexer in Hunnu) | [ ] | Not started (#43) |
| 4 | Documentation + language spec | [/] | Partial (outdated, #47-#52) |
| 5 | CI/CD (GitHub Actions) | [ ] | Not started (#44) |
| 6 | Benchmark suite | [ ] | Not started (#45) |
| 7 | v1.0 release | [ ] | Not started (#46) |

---

## Post-v1.0: Vision alignment (from hunnu-opencode-prompt.md)

| # | Feature | Status | Notes |
|---|---------|--------|-------|
| 1 | `def`/`end` block syntax | [ ] | Ruby-style blocks (#54) |
| 2 | String interpolation (`#{}`) | [ ] | (#55) |
| 3 | Range patterns + array destructuring in match | [ ] | (#56) |
| 4 | Option/Result types | [ ] | No null (#57) |
| 5 | `and`/`or`/`not` keyword operators | [ ] | (#58) |
| 6 | Stdlib API parity | [ ] | upcase, include?, first, etc. (#59) |
| 7 | Symbol type (`:ok`, `:err`) | [ ] | (#60) |

---

## Technical Debt / Code Health |

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
