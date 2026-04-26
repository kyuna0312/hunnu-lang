# Hunnu

A lightweight, expression-oriented programming language written in C.

---

## Features

| Feature | Syntax | Example |
|---------|--------|---------|
| Variables | `let x = 10` | `let x = 10` |
| Functions | `fn add(a, b) { return a + b }` | `fn add(a, b) { return a + b }` |
| If / Else / else if | `if x > 0 { ... } else if x > 5 { ... } else { ... }` | `if x > 0 { ... } else { ... }` |
| While loop | `while x > 0 { ... }` | `while i < 10 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Arithmetic | `+` `-` `*` `/` `%` | `x + y * z` |
| Comparison | `==` `!=` `<` `<=` `>` `>=` | `if x > 0 { ... }` |
| Boolean logic | `and` `or` `not` | `if a and b { ... }` |
| Print | `print("Hello")` | `print("Hello")` |
| Compound assignment | `x += 1`, `x -= 2`, `x *= 3`, `x /= 4` | `x += 5` |
| Arrays | `let arr = [1, 2, 3]` | `let arr = [1, 2, 3]` |
| Array access | `arr[0]` | `arr[0]` |
| String concat | `"a" + "b"` | `"Hello " + "World"` |
| String escapes | `"Hello\nWorld"`, `"Tab\there"` | `"Hello\nWorld"` |
| String len | `len(s)` | `len(s)` |
| Floats | `let pi = 3.14159` | `let pi = 3.14` |
| null/nil | `let x = null` | `let x = null` |

### Mongolian Keywords

Hunnu supports both English and Mongolian keywords:

| English | Mongolian (Cyrillic) |
|---------|---------------------|
| `let` | `хувьсагч` |
| `fn` | `функц` |
| `if` | `хэрвээ` |
| `true` | `үнэн` |
| `false` | `худал` |
| `print` | `хэвлэх` |
| `while` | `давталт` |
| `for` | `тооллого` |
| `return` | `буцаах` |
| `break` | `зогсоох` |
| `continue` | `үргэлжлүүлэх` |
| `null` | `хоосон` |

---

## Building

```bash
mkdir build && cd build
cmake ..
make
```

---

## Running

```bash
./build/hunnu run examples/main.hn
./build/hunnu run examples/main.hn --vm
./build/hunnu build examples/main.hn
./build/hunnu run examples/main.hn --debug
```

---

## Examples

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
    print(numbers[0])
    print(numbers[4])
}
```

### Mongolian Keywords

```hunnu
функц main() {
    хувьсагч тоо = 10
    хувьсагч бусад = 50
    хэвлэх(тоо)
    
    хэрвээ тоо > 5 {
        хэвлэх("greater than 5")
    } бусад {
        хэвлэх("less than 5")
    }
}
```

---

## Project Structure

```
hunnu-lang/
├── compiler/
│   ├── lexer/          # Tokenizer
│   ├── parser/        # Parser (AST)
│   ├── ast/          # AST node definitions
│   ├── interpreter/  # Tree-walk interpreter
│   └── vm/          # Bytecode + VM
├── cli/              # CLI
├── examples/         # Example code
└── CMakeLists.txt
```

---

## Roadmap

See [`plan.md`](plan.md) for the full development roadmap.

### Completed
- ✅ Phase 1: Foundation Fixes (scoping, break/continue, memory fixes)
- ✅ Phase 2: Core Language Features (compound assignment, else if, floats, null/nil, string escapes)
- ✅ Phase 3: Standard Library & Dev Experience (input, to_str/to_int/to_float, --debug)
- ✅ Phase 4: Bytecode Compiler + VM (build command, --vm flag)

---

## License

MIT