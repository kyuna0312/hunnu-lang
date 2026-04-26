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

### For Loop with Break and Continue

```hunnu
fn main() {
    let i = 0
    let count = 0
    while i < 10 {
        i = i + 1
        if i == 5 { continue }
        if i == 8 { break }
        count = count + 1
    }
    print(count)  // 6
}
```

### Compound Assignment

```hunnu
fn main() {
    let x = 10
    x += 5
    x -= 3
    x *= 2
    print(x)  // 24
}
```

### else if Chains

```hunnu
fn main() {
    let score = 85
    if score >= 90 {
        print("A")
    } else if score >= 80 {
        print("B")
    } else if score >= 70 {
        print("C")
    } else {
        print("F")
    }
}
```

### Floating Point

```hunnu
fn main() {
    let pi = 3.14159
    let r = 2.0
    print(pi * r * r)  // 12.5664
    print(10 + 3.5)    // 13.5
}
```

### String Escapes

```hunnu
fn main() {
    print("Hello\nWorld")       // newline
    print("Tab\there")           // tab
    print("Quote: \"test\"")      // escaped quote
    print("Backslash: \\")       // escaped backslash
}
```

### Scoped Variables

```hunnu
fn main() {
    let x = 10
    {
        let x = 20          // shadows outer x
        print(x)            // 20
    }
    print(x)                // 10
}
```

### Functions

```hunnu
fn add(a, b) {
    return a + b
}

fn main() {
    let result = add(5, 3)
    print(result)
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

### Strings

```hunnu
fn main() {
    let greeting = "Hello, "
    let name = "World"
    let message = greeting + name
    print(message)
    
    let s = "Hunnu"
    print(len(s))
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