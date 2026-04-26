# Өөрчлөлийн тэмдэглэг

> Хүмүүн хэлний өөрчлөлүүд.

---

## [Хэзээ ч тодорхойгүй]

### ✅ Phase 4: Bytecode + VM
*2025 оны 4 сар*

#### Bytecode заавар
- `opcodes.h` — заавар цуглуулага
- 常量, арифметик, харьцуулалт, удирдлага
- Хувьсагч хандалт, массив, индекс, функц дуудах

#### Bytecode компилятор
- `compiler/compiler.c` — AST → bytecode
- AST гүйлдэж bytecode гаргах
- Тогтмол хотолбор (strings)
- Програм, функц, блок, хувьсагч, if/while, массив, print

#### Виртуал машин
- `vm.c` — стек дээр суурилсан VM
- Удамлах стек (256 слот)
- Хувьсагч хадгалалт
- Суурилсан функц (print, input, to_int, to_float, to_str)
- Bytecode тайлбарлах давталт

#### CLI нэгдэл
- `build` команды — bytecode гаргах
- `--vm` туг — VM-ээр ажиллуулах

```bash
./hunnu build examples/main.hn
./hunnu run examples/main.hn --vm
```

#### Файлууд
- `compiler/vm/opcodes.h`
- `compiler/vm/compiler.h`
- `compiler/vm/compiler.c`
- `compiler/vm/vm.h`
- `compiler/vm/vm.c`
- `cli/main.c`, `cli/cli.h`

---

### ✅ Phase 3: Стандарт сан + DX
*2025 оны 4 сар*

#### CLI
- `--debug` — токен, AST хэвлэх
- `--help` — тусагдах

---

### ✅ Phase 2: Түлхүүр онцлогууд
*2025 оны 4 сар*

#### Нийлэх оператор
- `+=`, `-=`, `*=`, `/=`
- Парсер дээр `x = x + y` болгон

```hunnu
let x = 10
x += 5
x -= 3
x *= 2
x /= 4
```

#### else if гинж
- Олон `else if` дэмжлэг

```hunnu
if x > 90 {
    print("A")
} else if x > 80 {
    print("B")
} else {
    print("C")
}
```

#### Бодит тоо
- `TOKEN_FLOAT_LITERAL` болон `VALUE_FLOAT`
- + - * / дэмжлэх int/float хольсон

```hunnu
let pi = 3.14159
let r = 2.0
print(pi * r * r)
print(10 + 3.5)
```

#### null/nil
- `TOKEN_NULL` болон `TOKEN_NIL_KEYWORD`

```hunnu
let x = null
let y = nil
```

#### Тэмдэгт орлоос
- `\n`, `\t`, `\\`, `\"`

```hunnu
print("Сайн\nДэлхий")
print("Tab\there")
print("Тэмдэгт: \"тест\"")
```

---

### ✅ Phase 1: Үндсэн засварууд
*2025 оны 4 сар*

#### Хувьсагч хүрээлэл
- Өмнө нь: глобаль нэрийн орон
- Дараа нь: хүрээллийн гинж

```hunnu
let x = 10
{
    let x = 20
    print(x)
}
print(x)
```

#### break/continue
- Сигнал туг ашиглах

```hunnu
let i = 0
while i < 10 {
    i = i + 1
    if i == 5 { break }
    if i == 3 { continue }
    print(i)
}
```

#### Санах ой засварууд
- Парсер дээрх холбогдох зааг
- Тэмдэгт гүнзгий хуулах
- Массив гээлтүүд

---

## Өмнөх онцлогууд
- Хувьсагач: `let x = 5`
- Арифметик: `+`, `-`, `*`, `/`, `%`
- Харьцуулалт: `>`, `<`, `>=`, `<=`, `==`, `!=`
- Логик: `and`, `or`, `not`
- if/else
- while давталт
- for давталт
- Функц: `fn нэр(параметр) { ... }`
- return
- print
- Дахин оноох
- Массив: `[1, 2, 3]` + `arr[i]`
- Тэмдэгт нийлэх: `"a" + "b"`
- len()
- Функц дуудах (нэр��эр)