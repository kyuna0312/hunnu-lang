# Hunnu Language — Development Plan

> A living document tracking the state, priorities, and vision for Hunnu.

---

## Features

Hunnu is a lightweight programming language written in C. It supports both **English** and **Mongolian** (Cyrillic) keywords.

### Working Features

| Feature | English | Mongolian |
|---------|---------|-----------|
| Variables | `let x = 5` | `хувьсагч x = 5` |
| Functions | `fn add(a, b) { return a + b }` | `функц нэмэх(a, b) { буцаах a + b }` |
| If/else | `if x > 0 { ... } else { ... }` | `хэрвээ x > 5 { ... } бусад { ... }` |
| While loop | `while i < 10 { ... }` | `давталт i < 10 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` | `тооллого хувьсагч i = 0; i < 3; i = i + 1 { ... }` |
| Print | `print("Hello")` | `хэвлэх("Сайн уу")` |
| Return | `return value` | `буцаах утга` |
| Break | `break` | `зогсоох` |
| Continue | `continue` | `үргэлжлүүлэх` |
| null/nil | `null`, `nil` | `хоосон` |

---

## CLI Usage

```bash
./hunnu run examples/main.hn
./hunnu run examples/main.hn --vm
./hunnu build examples/main.hn
./hunnu run examples/main.hn --debug
```

---

## Phases

### Phase 1: Foundation Fixes ✅

| # | Feature | Description |
|----|---------|-------------|
| 1 | Variable scoping (scope stack) | Block-scoped variables |
| 2 | break/continue | Loop control flow |
| 3 | Array bounds checking | `arr[i]` IndexError |
| 4 | String memory safety | Dangling pointer fixes |

### Phase 2: Core Language Features ✅

| # | Feature | Description |
|----|---------|-------------|
| 1 | Compound assignment: `+=`, `-=`, etc | `x += 1` |
| 2 | `else if` chains | Multiple conditions |
| 3 | Floating-point numbers | `3.14`, `2.0` |
| 4 | `null`/`nil` literal | `let x = null` |

### Phase 3: Standard Library + DX ✅

| # | Feature | Description |
|----|---------|-------------|
| 1 | `input()` | Read from stdin |
| 2 | `to_int()`, `to_float()`, `to_str()` | Type conversions |
| 3 | `--debug` flag | Show tokens and AST |

### Phase 4: Bytecode + VM ✅

| # | Feature | Description |
|----|---------|-------------|
| 1 | Bytecode compiler | AST → bytecode |
| 2 | Virtual Machine | Bytecode execution |
| 3 | `build` command | Output bytecode |
| 4 | `--vm` flag | Run with VM |

---

## Next Steps

### High Priority

| Feature | Description |
|---------|-------------|
| Modules/`import` | Split code across files |
| Standard library | Common functions |

### Medium Priority

| Feature | Description |
|---------|-------------|
| Structs/Records | `type Point = { x: int, y: int }` |
| Pattern matching | `match x { ... }` |
| ADT (Sum types) | `type Maybe[T] = Just(T) \| Nothing` |

### Long-term Vision

| Feature | Description |
|---------|-------------|
| Self-hosting | Write compiler in Hunnu |
| JIT | Just-in-time compilation |
| AOT | Binary output |

---

## File Structure

```
hunnu-lang/
├── compiler/
│   ├── lexer/          # Tokenizer
│   ├── parser/        # Parser (AST)
│   ├── ast/          # AST node types
│   ├── interpreter/  # Tree-walk interpreter
│   └── vm/          # Bytecode + VM
├── cli/              # CLI
├── examples/         # Example code
└── CMakeLists.txt
```

---

## Development Timeline

```
2025-04  Phase 1: Foundation Fixes
2025-04  Phase 2: Core Language Features
2025-04  Phase 3: Standard Library + DX
2025-04  Phase 4: Bytecode + VM
```

---

## Links

- Web: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang

MIT License © 2025 Hunnu