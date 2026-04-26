# Hunnu

A lightweight, expression-oriented programming language written in C.

## Features / Онцлогууд

| Feature | Syntax | Синтакс |
|---------|--------|---------|
| Variables | `let x = 10` | `let x = 10` |
| Functions | `fn add(a, b) { return a + b }` | `fn add(a, b) { return a + b }` |
| If / Else | `if x > 0 { ... } else { ... }` | `if x > 0 { ... } else { ... }` |
| While loop | `while x > 0 { ... }` | `while x > 0 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Break / Continue | `break` / `continue` | `break` / `continue` |
| Arithmetic | `+` `-` `*` `/` `%` | `+` `-` `*` `/` `%` |
| Comparison | `==` `!=` `<` `<=` `>` `>=` | `==` `!=` `<` `<=` `>` `>=` |
| Boolean logic | `and` `or` `not` | `and` `or` `not` |
| Print | `print("Hello")` | `print("Hello")` |
| Arrays | `let arr = [1, 2, 3]` | `let arr = [1, 2, 3]` |
| Array access | `arr[0]` | `arr[0]` |
| String concat | `"a" + "b"` | `"a" + "b"` |
| String len | `len(s)` | `len(s)` |

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running / Ажиллуулах

```bash
./build/hunnu run examples/main.hn
```

## Examples / Жишээнүүд

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

### Arrays / Массив

```hunnu
fn main() {
    let numbers = [10, 20, 30, 40, 50]
    print(numbers[0])
    print(numbers[4])
}
```

### Strings / Тэмдэгт

```hunnu
fn main() {
    let greeting = "Сайн уу, "
    let name = "Дэлхий"
    let message = greeting + name
    print(message)
    
    let s = "Hunnu"
    print(len(s))
}
```

---

## Mongolian / Монгол хэл

### Хэрхэн ашиглах

**Hunnu** нь C хэл дээр бичигдсэн, програмчлалын хэл юм.

### Синтакс

```hunnu
// Хувьсагч зарлах
let x = 10

// Функц тодорхойлох
fn greeting(name) {
    return "Сайн уу, " + name
}

// Хэвлэх
print("Hello World")

// Нөхцөл шалгах
if x > 5 {
    print("Tom")
} else {
    print("Bhut")
}

// While давталт
let i = 0
while i < 10 {
    print(i)
    i = i + 1
}

// For давталт
for let i = 0; i < 5; i = i + 1 {
    print(i)
}

// Массив
let arr = [1, 2, 3, 4, 5]
print(arr[0])

// Тэмдэгт нийлүүлэх
let s1 = "Сайн "
let s2 = "уу"
let s3 = s1 + s2
print(s3)

// Тэмдэгт урт
let s = "Hunnu"
print(len(s))
```

### Жишээ програм

```hunnu
// Фибоначчи функц
fn fib(n) {
    if n <= 1 {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}

fn main() {
    // 10-р FIBONAЧИ тоо
    print(fib(10))
    
    // Массив ашиглах
    let numbers = [10, 20, 30, 40, 50]
    print(numbers[0])
    
    // Тэмдэгт ажиллах
    let name = "Hunnu"
    let message = "Programming Language: " + name
    print(message)
    print(len(message))
}
```

---

## Project Structure / Төслийн бүтэц

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

## Roadmap / Зорилтууд

See [`future-improvements.md`](future-improvements.md) for the full list. Near-term priorities:

- Scoped variable environments (function-local variables)
- First-class function calls
- Arrays and string operations
- Better runtime error messages with line numbers

## License

MIT
