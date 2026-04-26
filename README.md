# Hunnu (Хүннү)

A lightweight, expression-oriented programming language written in C.
Supports both English and Mongolian (Cyrillic) keywords.

---

## Version

**Current: 0.2.0 (Алтангэрэл)** - Үндсэн функцууд

Hunnu uses authentic Mongolian women names for versioning.
See [`compiler/version.h`](compiler/version.h) for the full version list.

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
| Input | `input()` | `let name = input()` |
| Type conversion | `to_int()`, `to_float()`, `to_str()` | `to_str(42)` |

### Built-in Functions

| Function | Description |
|----------|-------------|
| `print(x)` | Print value to stdout |
| `input()` | Read line from stdin |
| `len(arr)` | Get array length |
| `to_int(x)` | Convert to integer |
| `to_float(x)` | Convert to float |
| `to_str(x)` | Convert to string |

### Mongolian Keywords

Hunnu supports both English and Mongolian (Cyrillic) keywords:

| English | Mongolian (Cyrillic) | Meaning |
|---------|---------------------|---------|
| `let` | `хувьсагч` | variable |
| `fn` | `функц` | function |
| `if` | `хэрвээ` | if |
| `else` | `бусад` | else |
| `true` | `үнэн` | true |
| `false` | `худал` | false |
| `print` | `хэвлэх` | print |
| `while` | `давталт` | while |
| `for` | `тооллого` | for |
| `return` | `буцаах` | return |
| `break` | `зогсоох` | break |
| `continue` | `үргэлжлүүлэх` | continue |
| `null` | `хоосон` | null |

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
./hunnu run examples/main.hn     # Run with interpreter
./hunnu run examples/main.hn --vm   # Run with VM
./hunnu build examples/main.hn  # Compile to bytecode
./hunnu run examples/main.hn --debug  # Debug mode
./hunnu tokens examples/main.hn  # Show tokens
./hunnu ast examples/main.hn    # Show AST
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
│   ├── ast/           # AST node definitions
│   ├── interpreter/   # Tree-walk interpreter
│   ├── version.h      # Version constants
│   └── vm/            # Bytecode + VM
├── cli/               # CLI
├── examples/          # Example code
├── plan.md            # Development roadmap
└── CMakeLists.txt
```

---

## Versioning

Hunnu uses authentic Mongolian women names for versioning.
Each version represents a milestone in the language development.

| Version | Name | Meaning |
|---------|------|---------|
| 0.1.0 | Алтан (Altan) | Golden |
| 0.2.0 | Алтангэрэл (Altangerel) | Golden light |
| 0.3.0 | Алтанцэцэг (Altantsetseg) | Golden flower |
| 0.4.0 | Анар (Anar) | Pomegranate |
| 0.5.0 | Батцэцэг (Battsetseg) | Strong flower |
| ... | ... | ... |
| 1.0.0 | Эрдэнэ (Erdene) | Jewel |

See [`compiler/version.h`](compiler/version.h) for the full list.

---

## Roadmap

See [`plan.md`](plan.md) for the full development roadmap.

### Completed
- ✅ Phase 1: Foundation Fixes (scoping, break/continue, memory fixes)
- ✅ Phase 2: Core Language Features (compound assignment, else if, floats, null/nil, string escapes)
- ✅ Phase 3: Standard Library & Dev Experience (input, to_str/to_int/to_float, --debug)
- ✅ Phase 4: Bytecode Compiler + VM (build command, --vm flag)

### Next Steps (High Priority)
- Array memory fix (deep copy, proper free)
- Import statement for external files
- Error line numbers in source

---

## License

MIT

---

## Links

- Web: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang