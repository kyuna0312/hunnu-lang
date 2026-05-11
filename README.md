# Hunnu (Хүннү)

A lightweight, expression-oriented programming language written in C with Rust AOT compiler support. Supports both English and Mongolian (Cyrillic) keywords.

## Quick Start

```bash
# Install (Linux/macOS)
./install.sh

# Install (Windows)
install.bat

# Or build manually
mkdir build && cd build && cmake .. && make

# Run a program
./build/hunnu run examples/main.hn
```

After installing, add to your shell profile:

```sh
export HUNNU_STDLIB_PATH="/usr/local/lib/hunnu"
export PATH="$PATH:/usr/local/bin"
```

## Features

| Feature | Syntax | Example |
|---------|--------|---------|
| Variables | `let x = 10` | `let x = 10` |
| Functions | `fn add(a, b) { return a + b }` | `fn add(a, b) { return a + b }` |
| If/Else/else if | `if x > 0 { ... } else { ... }` | `if x > 0 { ... } else { ... }` |
| While loop | `while x > 0 { ... }` | `while i < 10 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Arrays | `let arr = [1, 2, 3]` | `arr[0]` |
| Structs | `type Point = { x: int, y: int }` | `let p = Point{10, 20}; print(p.x)` |
| Pointers | `&x`, `*p` | `let p = &x; print(*p)` |
| Match | `match val { 0 -> "zero", _ -> "other" }` | `match x { 1 -> "one", _ -> "many" }` |
| Classes | `class Point { pub x, pub y, fn new(self, x, y) { ... } }` | `let p = new Point(3, 4)` |
| Inheritance | `class Dog : Animal { ... }` | `class Dog : Animal { fn speak(self) { print("Woof!") } }` |
| Traits | `trait Area { fn area(self) }` | `impl Area for Circle { fn area(self) { ... } }` |
| Encapsulation | `pub` keyword | `pub x: int` (visible), `x` (private) |
| FFI | `extern fn puts(s) -> int from "libc.so.6"` | `extern fn pow(x,y) -> float from "libm.so.6"` |
| Try/Catch | `try { } catch { }` | `try { x / 0 } catch { print("error") }` |
| Module imports | `import std.math` | `import std.math` |
| AOT compilation | `hunnu compile file.hn -o output` | `hunnu compile main.hn -o main` |
| i18n | `--lang mn`, `HUNNU_LANG=mn` | Mongolian keywords + error messages |

### Built-in Functions

`print(x)` `input()` `len(arr)` `to_int(x)` `to_float(x)` `to_str(x)`

### Mongolian Keywords

| English | Mongolian (Cyrillic) | Translation |
|---------|---------------------|-------------|
| `let` | `хувьсагч` | variable |
| `fn` | `функц` | function |
| `if` / `else` | `хэрвээ` / `бусад` | if / else |
| `true` / `false` | `үнэн` / `худал` | true / false |
| `while` | `давталт` | loop |
| `for` | `тооллого` | iterate |
| `return` / `break` / `continue` | `буцаах` / `зогсоох` / `үргэлжлүүлэх` | return / stop / continue |
| `print` | `хэвлэх` | print |
| `import` | `импорт` | import |
| `try` / `catch` | `турших` / `барих` | try / catch |
| `match` | `тохирох` | match |
| `type` | `төрөл` | type / struct |
| `class` / `new` | `класс` / `шинэ` | class / new |
| `pub` / `self` | `нийт` / `өөрөө` | public / self |
| `trait` / `impl` | `ers` / `хэрэгжүүлэх` | trait / implement |

## Usage

```bash
hunnu run examples/main.hn           # Run with interpreter
hunnu run examples/main.hn --vm      # Run with C bytecode VM
hunnu run examples/main.hn --vm-rust # Run with Rust VM
hunnu run examples/main.hn --debug   # Debug mode (tokens + AST)
hunnu build examples/main.hn         # Compile to bytecode
hunnu compile main.hn -o main        # AOT compile to native binary
hunnu tokens examples/main.hn        # Show token stream
hunnu ast examples/main.hn           # Show AST
```

## Standard Library

Modules in `stdlib/`:

| Module | Description |
|--------|-------------|
| `std.libc` | C library FFI bindings (puts, printf, strlen, etc.) |
| `std.math` | Math functions (pow, sqrt, sin, cos, tan, fabs) |
| `std.io` | I/O functions (println, read_line) |
| `std.array` | Array utilities (map, filter, reduce, length) |
| `std.string` | String utilities (to_upper, to_lower, contains, trim, split, join) |
| `std.fs` | Filesystem functions (exists, read_file, write_file) |
| `std.time` | Time functions (now, timestamp) |

```hunnu
import std.math

fn main() {
    let x = 2.0
    print(pow(x, 3.0))
}
```

## Examples

```hunnu
// Hello World
fn main() {
    print("Hello, World!")
}

// Fibonacci
fn fib(n) {
    if n <= 1 { return n }
    return fib(n - 1) + fib(n - 2)
}

fn main() {
    print(fib(10))  // 55
}

// Try/Catch
fn main() {
    try {
        let x = 10 / 0
    } catch {
        print("Caught an error!")
    }
}

// FFI
extern fn pow(base, exp) -> float from "libm.so.6"

fn main() {
    print(pow(2.0, 3.0))  // 8.0
}

// Classes
class Point {
    pub x: int
    pub y: int
    fn new(self, x, y) {
        self.x = x
        self.y = y
    }
    fn length(self) {
        return self.x * self.x + self.y * self.y
    }
}
fn main() {
    let p = new Point(3, 4)
    print(p.length())  // 25
}

// Inheritance
class Dog : Animal {
    fn speak(self) { print("Woof!") }
}

// Traits
trait Area { fn area(self) }
impl Area for Circle { fn area(self) { return self.radius * self.radius * 3 } }

// Mongolian keywords
функц main() {
    хувьсагч тоо = 10
    хэрвээ тоо > 5 {
        хэвлэх("greater than 5")
    }
}
```

## Project Structure

```
hunnu-lang/
├── compiler/           # C interpreter and bytecode VM
│   ├── lexer/         # Tokenizer
│   ├── parser/        # Parser (AST builder)
│   ├── ast/           # AST node definitions
│   ├── interpreter/   # Tree-walk interpreter
│   └── vm/            # Bytecode compiler + VM
├── compiler-rust/     # Rust AOT compiler frontend (LLVM)
├── vm-rust/           # Rust bytecode VM (staticlib + binary)
├── cli/               # Command-line interface
├── stdlib/            # Standard library modules
├── bindings/python/   # Python bindings (PyO3)
├── examples/          # Example .hn programs
├── install.sh         # Linux/macOS installer
├── install.bat        # Windows installer
└── CMakeLists.txt     # Build configuration
```

## Versioning

Hunnu uses authentic Mongolian women names for versioning (`compiler/version.h`).

| Version | Name | Meaning |
|---------|------|---------|
| 0.1.0 | Алтан (Altan) | Golden |
| 0.2.0 | Алтангэрэл (Altangerel) | Golden light |
| ... | ... | ... |
| **1.0.0** | **Эрдэнэ (Erdene)** | **Jewel** |
| 1.1.0 | Эрдэнэчимэг (Erdenechimeg) | Jewel ornament |

## Roadmap

See [`plan.md`](plan.md) for the full roadmap.

### Completed
- Foundation fixes (scoping, break/continue, memory)
- Core features (compound assignment, else if, floats, string escapes)
- Standard library and dev experience (input type conversion, debug mode)
- Bytecode compiler + VM (build command, --vm flag)
- FFI ecosystem (libc.hn, try/catch, std modules, Python bindings)
- AOT compiler foundation (structs, pointers, Rust frontend, LLVM skeleton)
- **OOP features** — classes, inheritance, polymorphism, encapsulation, traits/impl, `self` reference
- **Pattern matching** — match expressions with literal, wildcard, and identifier patterns
- **i18n** — Mongolian keywords, `--lang mn` flag, localized error messages

### Next Steps
- Complete LLVM codegen in compiler-rust
- Enums / ADTs, generics, unsafe blocks
- Functional programming: immutability, first-class functions, closures, lambdas
- Package manager (`hunnu install`, `hunnu new`)
- Self-hosting: write Hunnu lexer in Hunnu
- CI/CD pipeline, benchmark suite, v1.0 release

## License

MIT

## Links

- Web: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang
