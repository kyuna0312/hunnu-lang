# Hunnu

A lightweight, expression-oriented programming language written in C.

## Features

| Feature | Syntax |
|---------|--------|
| Variables | `let x = 10` |
| Functions | `fn add(a, b) { return a + b }` |
| If / Else | `if x > 0 { ... } else { ... }` |
| While loop | `while x > 0 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Break / Continue | `break` / `continue` inside loops |
| Arithmetic | `+` `-` `*` `/` `%` |
| Comparison | `==` `!=` `<` `<=` `>` `>=` |
| Boolean logic | `and` `or` `not` |
| Print | `print("Hello")` |

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running

```bash
./build/hunnu run examples/main.hn
```

## Examples

### Hello World

```hunnu
fn main() {
    print("Hello from Hunnu!")
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
    print(fib(10))
}
```

### For Loop

```hunnu
fn main() {
    for let i = 0; i < 5; i = i + 1 {
        print(i)
    }
}
```

### While Loop with Break

```hunnu
fn main() {
    let x = 10
    while x > 0 {
        if x == 5 {
            break
        }
        print(x)
        x = x - 1
    }
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

## Project Structure

```
hunnu-lang/
├── compiler/
│   ├── ast/          # AST node definitions and constructors
│   ├── interpreter/  # Tree-walk runtime
│   ├── lexer/        # Tokenizer
│   └── parser/       # Recursive-descent parser
├── cli/              # Entry point and CLI handling
├── examples/         # Sample .hn programs
└── CMakeLists.txt
```

## Roadmap

See [`future-improvements.md`](future-improvements.md) for the full list. Near-term priorities:

- Scoped variable environments (function-local variables)
- First-class function calls
- Arrays and string operations
- Better runtime error messages with line numbers

## License

MIT
