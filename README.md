# Hunnu

A lightweight, expression-oriented programming language written in C.

## Features / Онцлогууд

| Feature | Syntax | Синтакс |
|---------|--------|---------|
| Variables | `let x = 10` | `let x = 10` |
| Functions | `fn add(a, b) { return a + b }` | `fn add(a, b) { return a + b }` |
| If / Else / else if | `if x > 0 { ... } else if x > 5 { ... } else { ... }` | `if x > 0 { ... } else if x > 5 { ... } else { ... }` |
| While loop | `while x > 0 { ... }` | `while x > 0 { ... }` |
| For loop | `for let i = 0; i < 3; i = i + 1 { ... }` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Break / Continue | `break` / `continue` | `break` / `continue` |
| Arithmetic | `+` `-` `*` `/` `%` | `+` `-` `*` `/` `%` |
| Comparison | `==` `!=` `<` `<=` `>` `>=` | `==` `!=` `<` `<=` `>` `>=` |
| Boolean logic | `and` `or` `not` | `and` `or` `not` |
| Print | `print("Hello")` | `print("Hello")` |
| Compound assignment | `x += 1`, `x -= 2`, `x *= 3`, `x /= 4` | `x += 1`, `x -= 2`, `x *= 3`, `x /= 4` |
| Arrays | `let arr = [1, 2, 3]` | `let arr = [1, 2, 3]` |
| Array access | `arr[0]` | `arr[0]` |
| String concat | `"a" + "b"` | `"a" + "b"` |
| String escapes | `"Hello\nWorld"`, `"Tab\there"` | `"Hello\nWorld"`, `"Tab\there"` |
| String len | `len(s)` | `len(s)` |
| Floats | `let pi = 3.14159` | `let pi = 3.14159` |
| null/nil | `let x = null` | `let x = null` |

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
    print(count)  // prints 6
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

See [`plan.md`](plan.md) for the full development roadmap.

### Completed
- ✅ Phase 1: Foundation Fixes (scoping, break/continue, memory fixes)
- ✅ Phase 2: Core Language Features (compound assignment, else if, floats, null/nil, string escapes)
- ✅ Phase 3: Standard Library & Dev Experience (input, to_str/to_int/to_float, --debug)
- ✅ Phase 4: Bytecode Compiler + VM (build command, --vm flag)

## License

MIT
