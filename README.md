# Хүмүүн хэл

> Хөнгөн, илэрхийлэлд төвлөсөн, C хэл дээр бичигдсэн програмчлалын хэл.

---

## онцлогууд

| онцлог | синтакс | жишээ |
|---------|---------|--------|
| хувьсагч | `let x = 5` | `let x = 5` |
| арифметик | `+` `-` `*` `/` `%` | `x + y * z` |
| харьцуулалт | `==` `!=` `<` `<=` `>` `>=` | `if x > 0` |
| логик | `and` `or` `not` | `if a and b` |
| нөхцөл | `if` / `else` / `else if` | `if x > 0 { ... } else { ... }` |
| давталт | `while` | `while i < 10 { ... }` |
| давталт | `for` | `for let i = 0; i < 3; i = i + 1 { ... }` |
| функц | `fn` | `fn add(a, b) { return a + b }` |
| буцаах | `return` | `return a + b` | 
| хэвлэх | `print` | `print("Сайн уу")` |
| массив | `[1, 2, 3]` | `let arr = [1, 2, 3]` |
| массив хандалт | `arr[i]` | `arr[0]` |
| тэмдэгт | `"a" + "b"` | `"Сайн " + "уу"` |
| урт | `len(s)` | `len("Хүмүүн")` |
| оруулах | `input()` | `let нэр = input()` |
| төрөл шилжүүлэх | `to_int()` `to_float()` `to_str()` | `to_int("42")` |
| бодит тоо | `3.14` | `let pi = 3.14` |
| хоосон | `null` / `nil` | `let x = null` |
| зогсоох | `break` | `break` |
| үргэлжлүүлэх | `continue` | `continue` |
| хүрээлэл | `{ ... }` | `{ let x = 10 ... }` |

---

## Барих

```bash
mkdir build && cd build
cmake ..
make
```

---

## Ажиллуулах

```bash
./build/hunnu run examples/main.hn
./build/hunnu run examples/main.hn --vm
./build/hunnu build examples/main.hn
./build/hunnu run examples/main.hn --debug
```

---

## Жишээнүүд

### Сайн уу дэлхий

```hunnu
fn main() {
    print("Сайн уу, Дэлхий!")
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
    print(fib(10))
}
```

### Массивтай ажиллах

```hunnu
fn main() {
    let тоонууд = [10, 20, 30, 40, 50]
    print(тоонууд[0])
    print(len(тоонууд))
}
```

### Хүрээлэл

```hunnu
fn main() {
    let x = 10
    {
        let x = 20
        print(x)
    }
    print(x)
}
```

### Давталт зогсоох

```hunnu
fn main() {
    let i = 0
    while i < 10 {
        i = i + 1
        if i == 5 {
            continue
        }
        if i == 8 {
            break
        }
        print(i)
    }
}
```

### Төрөл шилжүүлэх

```hunnu
fn main() {
    let тоо = to_int("42")
    print(тоо + 1)

    let бодит = to_float("3.14")
    print(бодит * 2)

    let тэмдэгт = to_str(123)
    print(тэмдэгт + "456")
}
```

---

## Бүтэц

```
hunnu-lang/
├── compiler/
│   ├── lexer/          # токенчлог
│   ├── parser/        # парсер
│   ├── ast/          # AST төрөл
│   ├── interpreter/  # ажиллуулах
│   └── vm/          # bytecode + VM
├── cli/              # команд мөр
├── examples/         # жишээ
└── CMakeLists.txt
```

---

## Төлөвлөгөө

### ✅ Phase 1: Үндсэн засварууд
- хувьсагч хүрээлэл
- break/continue
- массив хязгаар
- санах ой засах

### ✅ Phase 2: Түлхүүр онцлогууд
- += -= *= /=
- else if
- бодит тоо
- null/nil

### ✅ Phase 3: Стандарт сан
- input()
- to_int/to_float/to_str
- --debug

### ✅ Phase 4: Bytecode + VM
- компилятор
- виртуал машин

---

MIT ✅ 2025 Хүмүүн