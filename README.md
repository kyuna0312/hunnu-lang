# Hunnu

A lightweight, expression-oriented programming language written in C.

## Features

- **Variables**: `let x = 10`
- **Functions**: `fn add(a, b) { return a + b }`
- **Control Flow**: `if`, `while`, `for` loops
- **Loop Control**: `break` and `continue`
- **Arithmetic**: Full support for `+`, `-`, `*`, `/`, `%`
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Boolean Logic**: `and`, `or`, `not`
- **Printing**: `print("Hello")`

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

### For Loop

```hunnu
fn main() {
    for let i = 0; i < 3; i = i + 1 {
        print(i)
    }
}
```

### While Loop with Break

```hunnu
fn main() {
    let x = 3
    while x > 0 {
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
│   ├── ast/          # Abstract syntax tree
│   ├── interpreter/  # Runtime execution
│   ├── lexer/        # Tokenization
│   └── parser/       # Syntax analysis
├── cli/              # Command-line interface
└── examples/         # Sample programs
```

## License

MIT