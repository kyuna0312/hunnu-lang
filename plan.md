# Hunnu хэл — Хөгжлийн төлөвлөгөө

> Hunnu-ийн одоогийн байдал, тэргүүлэх ажлууд болон алсын хараа.

---

## Одоогийн төлөв

### Ажиллаж байгаа онцлогууд

| Онцлог | Синтакс | Жишээ |
|--------|---------|--------|
| Хувьсагч | `let x = 5` | `let x = 5` |
| Арифметик | `+`, `-`, `*`, `/`, `%` | `x + y * z` |
| Харьцуулалт | `>`, `<`, `>=`, `<=`, `==`, `!=` | `if x > 0 { ... }` |
|，布л | `and`, `or`, `not` | `if a and b { ... }` |
| `if` / `else` | `if x > 0 { ... } else { ... }` | `if x > 0 { " их " } else { " бага " }` |
| `else if` | `else if` гинж | `else if x > 5 { ... } else { ... }` |
| `while` давталт | `while(condition) { body }` | `while i < 10 { i = i + 1 }` |
| `for` давталт | `for(init; condition; update) { body }` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| Функц | `fn name(params) { body }` | `fn add(a, b) { return a + b }` |
| `return` | `return expression` | `return a + b` |
| `print` | `print(value)` | `print("Hello")` |
| Дахин оноох | `x = new_value` | `x = 10` |
| `+=`, `-=`, `*=`, `/=` | `x += 1` | `x += 5` |
| Массив | `[1, 2, 3]` | `let arr = [1, 2, 3]` |
| Массив хандалт | `arr[i]` | `arr[0]` |
| Тэмдэгт нийлэх | `"a" + "b"` | `"Hello " + "World"` |
| Тэмдэгт орлоос | `\n`, `\t`, `\\`, `\"` | `"Hello\nWorld"` |
| `len()` | `len(s)` | `len("abc")` |
| `input()` | `input()` | `let name = input()` |
| `to_int()` | `to_int(s)` | `to_int("42")` |
| `to_float()` | `to_float(s)` | `to_float("3.14")` |
| `to_str()` | `to_str(n)` | `to_str(42)` |
| `float` тоо | `3.14159` | `let pi = 3.14` |
| `null`/`nil` | `let x = null` | `let x = nil` |
| `break` | `break` | `while i < 10 { if i == 5 { break } }` |
| `continue` | `continue` | `while i < 10 { i = i + 1; if i == 3 { continue } }` |
| Хүрээлэл | `{ let x = 10 ... }` | Хувьсагч хүрээлэх |

---

## CLI хэрэглээ

```bash
# Эх код ажиллуулах (interpreter)
./hunnu run examples/main.hn

# VM-ээр ажиллуулах
./hunnu run examples/main.hn --vm

# Bytecode гаргах
./hunnu build examples/main.hn

# Токенүүд хэвлэх (debug)
./hunnu run examples/main.hn --debug

# AST хэвлэх (debug)
./hunnu ast examples/main.hn
```

---

## Фаз

### Phase 1: Үндсэн засварууд ✅
*2025 оны 4-р сар*

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | Хувьсагч хүрээлэл (scope stack) | `{ ... }` дотор хувьсагч |
| 2 | `break`/`continue` | Давталтаас гарах |
| 3 | Массив хязгаар шалгах | `arr[i]` IndexError |
| 4 | Тэмдэгт санах ой | Dangling pointer засах |

**Файлууд:** `interpreter.c`, `parser.c`

### Phase 2: Түлхүүр онцлогууд ✅
*2025 оны 4-р сар*

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | `+=`, `-=`, `*=`, `/=` | Нийлэх, хасах, үржүүлэх, хуваах |
| 2 | `else if` гинж | Олон нөхцөл шалгах |
| 3 | `float` тоо | `3.14`, `2.0` |
| 4 | `null`/`nil` | `let x = null` |

**Файлууд:** `lexer.c`, `token.h`, `parser.c`, `interpreter.c`

### Phase 3: Стандарт сан + DX ✅
*2025 оны 4-р сар*

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | `input()` | Стандарт оролдсон |
| 2 | `to_int()`, `to_float()`, `to_str()` | Төрөл шилжүүлэх |
| 3 | `--debug` | Токен, AST хэвлэх |

**Файлууд:** `interpreter.c`, `cli/main.c`

### Phase 4: Bytecode + VM ✅
*2025 оны 4-р сар*

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | Bytecode компилятор | AST → bytecode |
| 2 | Virtual Machine | Bytecode ажиллуулах |
| 3 | `build` команды | Bytecode гаргах |
| 4 | `--vm` туг | VM ажиллуулах |

**Файлууд:** `compiler/vm/`

---

## Дараагийн алхсууд

### Түлхүүр (2025)

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | Модуль/`import` | Файлууд хооронд импорт |
| 2 | Стандарт сан | нийтлэг функцүүд |

### Дунд (2026+)

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | Struct/Record | `type Point = { x: int, y: int }` |
| 2 | Pattern matching | `match x { ... }` |
| 3 | ADT (Sum type) | `type Maybe[T] = Just(T) \| Nothing` |

### Алсын хараа

| # | Онцлог | Тайлбар |
|----|--------|---------|
| 1 | Self-hosting | Хэлээ өөрөө бичигдсэн |
| 2 | JIT | JIT компиляц |
| 3 | AOT | Бинар гаралт |

---

## Жишээ код

### Hello World

```hunnu
fn main() {
    print("Hello, World!")
}
```

### Фибоначчи

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

### Массивтай ажиллах

```hunnu
fn main() {
    let numbers = [10, 20, 30, 40, 50]
    print(numbers[0])  // 10
    print(len(numbers))  // 5
}
```

### While давталт

```hunnu
fn main() {
    let i = 0
    let sum = 0
    while i < 10 {
        i = i + 1
        sum = sum + i
    }
    print(sum)  // 55
}
```

### break/continue

```hunnu
fn main() {
    let i = 0
    let count = 0
    while i < 10 {
        i = i + 1
        if i == 3 { continue }
        if i == 8 { break }
        count = count + 1
    }
    print(count)  // 6
}
```

### Төрөл шилжүүлэх

```hunnu
fn main() {
    let x = "42"
    let n = to_int(x)
    print(n + 1)  // 43

    let f = to_float("3.14")
    print(f * 2)  // 6.28

    let s = to_str(123)
    print(s + "456")  // 123456
}
```

---

## Файлын бүтэц

```
hunnu-lang/
├── compiler/
│   ├── lexer/          # Токенчлог
│   │   ├── lexer.c
│   │   └── token.h
│   ├── parser/        # Парсер (AST)
│   │   ├── parser.c
│   │   └── parser.h
│   ├── ast/          # AST төрөл
│   │   ├── ast.c
│   │   └── ast.h
│   ├── interpreter/  # Уламжлалт тоглогч
│   │   ├── interpreter.c
│   │   └── interpreter.h
│   └── vm/          # Bytecode + VM
│       ├── opcodes.h
│       ├── compiler.c
│       ├── compiler.h
│       ├── vm.c
│       └── vm.h
├── cli/              # CLI
│   ├── main.c
│   └── cli.h
├── examples/         # Жишээ код
└── CMakeLists.txt
```

---

## Хөгжлийн түүх

```
2025-04  Phase 1: Үндсэн засварууд
2025-04  Phase 2: Түлхүүр онцлогууд
2025-04  Phase 3: Стандарт сан + DX
2025-04  Phase 4: Bytecode + VM
```

---

## Холбоо

- Вэб: https://hunnu-lang.dev
- GitHub: https://github.com/hunnu-labs/hunnu-lang

MIT License © 2025 Hunnu