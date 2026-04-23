# Hunnu

A lightweight, expression-oriented programming language written in C.

## Features

- **Variables**: `let x = 10`
- **Functions**: `fn add(a, b) { return a + b }`
- **Control Flow**: `if`, `while`, `for` loops
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

## Example

```hunnu
fn main() {
    let x = 10

    if x > 5 {
        print("Hello from Hunnu!")
    }

    let sum = 5 + 3
    print(sum)
}
```

## Project Structure

```
compiler/
  ast/          # Abstract syntax tree
  interpreter/  # Runtime execution
  lexer/        # Tokenization
  parser/       # Syntax analysis
cli/            # Command-line interface
examples/       # Sample programs
```

## License

MIT