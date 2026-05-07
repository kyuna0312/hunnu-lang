# Hunnu (Хүннү)

A lightweight, expression-oriented programming language written in C.
Supports both English and Mongolian (Cyrillic) keywords.

---

## Version

**Current: 0.3.0 (Солонго)** - AOT Compiler Foundation

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
| **Try/Catch** | `try { } catch { }` | `try { x / 0 } catch { print("error") }` |
| **FFI (Foreign Function Interface)** | `extern fn puts(s) -> int from "libc.so.6"` | `extern fn pow(x,y) -> float from "libm.so.6"` |
| **Module imports** | `import std.math` | `import std.math` |
| **Structs/Records** | `type Point = { x: int, y: int }` | `type Point = { x, y }` |
| **Field access** | `point.x` | `let p = Point{10, 20}; print(p.x)` |
| **Pointers** | `&x`, `*p` | `let p = &x; print(*p)` |
| **AOT Compilation** | `hunnu compile file.hn -o output` | `hunnu compile main.hn -o main` |

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

| English | Mongolian (Cyrillic) | Translation |
|---------|---------------------|--------------|
| `let` | `хувьсагч` | variable |
| `fn` | `функц` | function |
| `if` | `хэрвээ` | if |
| `else` | `бусад` | else |
| `true` | `үнэн` | true |
| `false` | `худал` | false |
| `print` | `хэвлэх` | print |
| `while` | `давталт` | repeat/loop |
| `for` | `тооллого` | count/iterate |
| `return` | `буцаах` | return |
| `break` | `зогсоох` | stop |
| `continue` | `үргэлжлүүлэх` | continue |
| `null` | `хоосон` | empty |
| `import` | `импорт` | import |
| `try` | `турших` | try |
| `catch` | `барих` | catch |
| `finally` | `эцэст` | finally |

---

## Standard Library

Hunnu includes a standard library in `stdlib/` with modules:

| Module | Description |
|--------|-------------|
| `std/libc.hn` | C standard library FFI bindings (puts, printf, strlen, etc.) |
| `std/math.hn` | Math functions (pow, sqrt, sin, cos, tan, fabs) |
| `std/io.hn` | I/O functions (println, read_line) |
| `std/array.hn` | Array utilities (map, filter, reduce, length) |
| `std/string.hn` | String utilities (to_upper, to_lower, contains, trim, split, join) |
| `std/fs.hn` | Filesystem functions (exists, read_file, write_file) |
| `std/time.hn` | Time functions (now, timestamp) |

Usage:
```hunnu
import std.math
import std.io

fn main() {
    let x = 2.0
    let result = pow(x, 3.0)
    std.io.println("Result: " + to_str(result))
}
```

---

## Building:

```bash
mkdir build && cd build
cmake ..
make
```

---

## Running:

```bash
./hunnu run examples/main.hn          # Run with interpreter
./hunnu run examples/main.hn --vm     # Run with C VM
./hunnu run examples/main.hn --vm-rust  # Run with Rust VM
./hunnu build examples/main.hn           # Compile to bytecode
./hunnu compile examples/main.hn -o main  # AOT compile to native binary
./hunnu run examples/main.hn --debug   # Debug mode
./hunnu tokens examples/main.hn         # Show tokens
./hunnu ast examples/main.hn            # Show AST
```

### Try/Catch Example:

```hunnu
fn main() {
    try {
        print("Inside try block")
        let x = 10 / 0  // This would cause an error
    } catch {
        print("Caught an error!")
    }
    
    print("Program continues...")
}
```

### FFI Example:

```hunnu
// Call C library functions
extern fn pow(base, exp) -> float from "libm.so.6"
extern fn getenv(name) -> str from "libc.so.6"

fn main() {
    let result = pow(2.0, 3.0)
    print(result)  // 8.0
    
    let home = getenv("HOME")
    print(home)
}
```

### Module Import Example:

```hunnu
import std.math
import std.io

fn main() {
    let x = 2.0
    let result = pow(x, 3.0)
    std.io.println("Result: " + to_str(result))
}
```

---

## Examples:

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
- ✅ Month 2: FFI Ecosystem + Standard Library (libc.hn, FFI strings/floats, try/catch, std modules, Python bindings)
- ✅ Month 3: AOT Compiler Foundation (structs, field access, pointers, Rust compiler frontend, LLVM codegen skeleton, `compile` command)

### Next Steps (High Priority)
- Complete LLVM codegen in compiler-rust/
- Implement struct field access in interpreter
- Add type checking pass
- Self-hosting: Write Hunnu lexer in Hunnu

---

## License

MIT

---

## Links

- Web: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang