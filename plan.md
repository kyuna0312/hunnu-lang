# Хүннү Language — Development Plan

> A living document tracking the state, priorities, and vision for Хүннү.

---

## Current Language State

### Working Features

| Feature | Syntax | Example |
|---------|--------|---------|
| Variables | `let x = 5` | `let x = 5` |
| Arithmetic | `+`, `-`, `*`, `/`, `%` | `x + y * z` |
| Comparison | `>`, `<`, `>=`, `<=`, `==`, `!=` | `if x > 0 { ... }` |
| Boolean | `and`, `or`, `not` | `if a and b { ... }` |
| If/else | `if x > 0 { ... } else { ... }` | `if x > 0 { "A" } else { "B" }` |
| else if | `else if` chains | `else if x > 5 { ... } else { ... }` |
| While loop | `while(condition) { body }` | `while i < 10 { i = i + 1 }` |
| For loop | `for(init; condition; update) { body }` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Functions | `fn name(params) { body }` | `fn add(a, b) { return a + b }` |
| Return | `return expression` | `return a + b` |
| Print | `print(value)` | `print("Hello")` |
| Variable reassignment | `x = new_value` | `x = 10` |
| Compound assignment | `+=`, `-=`, `*=`, `/=` | `x += 1` |
| Arrays | `[1, 2, 3]` | `let arr = [1, 2, 3]` |
| Array access | `arr[i]` | `arr[0]` |
| String concat | `"a" + "b"` | `"Hello " + "World"` |
| String escapes | `\n`, `\t`, `\\`, `\"` | `"Hello\nWorld"` |
| `len()` | `len(s)` | `len("abc")` |
| `input()` | `input()` | `let name = input()` |
| `to_int()` | `to_int(s)` | `to_int("42")` |
| `to_float()` | `to_float(s)` | `to_float("3.14")` |
| `to_str()` | `to_str(n)` | `to_str(42)` |
| Float | `3.14159` | `let pi = 3.14` |
| null/nil | `null` / `nil` | `let x = null` |
| break | `break` | `while i < 10 { if i == 5 { break } }` |
| continue | `continue` | `while i < 10 { i = i + 1; if i == 3 { continue } }` |
| Scoping | `{ ... }` | Block-scoped variables |

---

## CLI Usage

```bash
# Run with interpreter
./hunnu run examples/main.hn

# Run with VM
./hunnu run examples/main.hn --vm

# Output bytecode
./hunnu build examples/main.hn

# Debug tokens and AST
./hunnu run examples/main.hn --debug

# Output AST only
./hunnu ast examples/main.hn
```

---

## Phases

### Phase 1: Foundation Fixes ✅
*April 2025*

| # | Feature | Description |
|----|--------|-------------|
| 1 | Variable scoping (scope stack) | Block-scoped variables |
| 2 | break/continue | Loop control flow |
| 3 | Array bounds checking | `arr[i]` IndexError |
| 4 | String memory safety | Dangling pointer fixes |

### Phase 2: Core Language Features ✅
*April 2025*

| # | Feature | Description |
|----|--------|-------------|
| 1 | Compound assignment: `+=`, `-=`, etc | `x += 1` |
| 2 | `else if` chains | Multiple conditions |
| 3 | Floating-point numbers | `3.14`, `2.0` |
| 4 | `null`/`nil` literal | `let x = null` |

### Phase 3: Standard Library + DX ✅
*April 2025*

| # | Feature | Description |
|----|--------|-------------|
| 1 | `input()` | Read from stdin |
| 2 | `to_int()`, `to_float()`, `to_str()` | Type conversions |
| 3 | `--debug` flag | Show tokens and AST |

### Phase 4: Bytecode + VM ✅
*April 2025*

| # | Feature | Description |
|----|--------|-------------|
| 1 | Bytecode compiler | AST → bytecode |
| 2 | Virtual Machine | Bytecode execution |
| 3 | `build` command | Output bytecode |
| 4 | `--vm` flag | Run with VM |

---

## Next Steps

### High Priority (2025)

| # | Feature | Description |
|----|--------|-------------|
| 1 | Modules/`import` | Split code across files |
| 2 | Standard library | Common functions |

### Medium Priority (2026+)

| # | Feature | Description |
|----|--------|-------------|
| 1 | Structs/Records | `type Point = { x: int, y: int }` |
| 2 | Pattern matching | `match x { ... }` |
| 3 | ADT (Sum types) | `type Maybe[T] = Just(T) \| Nothing` |

### Long-term Vision

| # | Feature | Description |
|----|--------|-------------|
| 1 | Self-hosting | Write compiler in Хүннү |
| 2 | JIT | Just-in-time compilation |
| 3 | AOT | Binary output |

---

## Code Examples

### Hello World

```hunnu
fn main() {
    print("Hello, World!")
}
```

### Fibonacci

```hunnu
fn fib(n) {
    if n <= 1 {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}

fn main() {
    print(fib(10))  // 55
}
```

### Arrays

```hunnu
fn main() {
    let numbers = [10, 20, 30, 40, 50]
    print(numbers[0])  // 10
    print(len(numbers))  // 5
}
```

### While Loop

```hunnu
fn main() {
    let i = 0
    let sum = 0
    while i < 10 {
        i = i + 1
        sum = sum + i
    }
    print(sum)  // 55
}
```

### break/continue

```hunnu
fn main() {
    let i = 0
    let count = 0
    while i < 10 {
        i = i + 1
        if i == 3 { continue }
        if i == 8 { break }
        count = count + 1
    }
    print(count)  // 6
}
```

### Type Conversions

```hunnu
fn main() {
    let x = "42"
    let n = to_int(x)
    print(n + 1)  // 43

    let f = to_float("3.14")
    print(f * 2)  // 6.28

    let s = to_str(123)
    print(s + "456")  // 123456
}
```

---

## File Structure

```
hunnu-lang/
├── compiler/
│   ├── lexer/          # Tokenizer
│   │   ├── lexer.c
│   │   └── token.h
│   ├── parser/        # Parser (AST)
│   │   ├── parser.c
│   │   └── parser.h
│   ├── ast/          # AST node types
│   │   ├── ast.c
│   │   └── ast.h
│   ├── interpreter/  # Tree-walk interpreter
│   │   ├── interpreter.c
│   │   └── interpreter.h
│   └── vm/          # Bytecode + VM
│       ├── opcodes.h
│       ├── compiler.c
│       ├── compiler.h
│       ├── vm.c
│       └── vm.h
├── cli/              # CLI
│   ├── main.c
│   └── cli.h
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

MIT License © 2025 Хүннү