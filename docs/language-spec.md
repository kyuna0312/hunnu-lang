# Hunnu Language Specification

Version 1.0.0 (Erdene)

Hunnu is a lightweight, expression-oriented programming language with C-level performance.
It supports functional, object-oriented, and procedural programming paradigms with
a clean, readable syntax. The language features a tree-walk interpreter (C) and an
AOT compiler (C transpiler + gcc, with an experimental Rust/LLVM frontend).

## Table of Contents

1. [Lexical Structure](#lexical-structure)
2. [Types](#types)
3. [Variables](#variables)
4. [Functions](#functions)
5. [Control Flow](#control-flow)
6. [Expressions](#expressions)
7. [Built-in Functions](#built-in-functions)
8. [Arrays](#arrays)
9. [Structs](#structs)
10. [Classes and OOP](#classes-and-oop)
11. [Traits and Impl](#traits-and-impl)
12. [Match Expressions](#match-expressions)
13. [Pattern Matching](#pattern-matching)
14. [Error Handling](#error-handling)
15. [Modules and Imports](#modules-and-imports)
16. [FFI (Foreign Function Interface)](#ffi-foreign-function-interface)
17. [Pointers and Unsafe](#pointers-and-unsafe)
18. [Enums](#enums)
19. [Internationalization](#internationalization)
20. [AOT Compilation](#aot-compilation)
21. [Command Line Interface](#command-line-interface)
22. [Grammar Reference](#grammar-reference)

---

## Lexical Structure

### Comments

Single-line comments start with `//`:

```hunnu
// This is a comment
let x = 10  // inline comment
```

### Identifiers

Identifiers start with a letter or underscore, followed by letters, digits, or underscores:

```hunnu
let myVar = 42
let _private = 100
let camelCase = 1
```

### Keywords

#### English
```
let      fn       if       else     while    for
return   print    true     false    not      and      or
break    continue match    null     nil      import
extern   try      catch    finally  type     class
new      pub      self     trait    impl     unsafe
enum     mut
```

#### Mongolian Cyrillic (--lang mn)
```
хувьсагч  функц    хэрвээ   бусад    давталт  тооллого
буцаах    хэвлэх   үнэн     худал    үгүй     мөн       эсвэл
зогсоох   үргэлжлүүлэх тохирох хоосон  импорт
гаднах    турших   барих    эцэст    төрөл    класс
шинэ      нийт     өөрөө    шинж     хэрэгжүүлэх аюулгүйбус
тоолол    өөрчлөгдөх
```

### Whitespace

Hunnu ignores whitespace (spaces, tabs, newlines) between tokens. Statements are
separated by newlines (semicolons are optional in most cases).

---

## Types

Hunnu has the following built-in types:

| Type | Description | Example |
|------|-------------|---------|
| `int` | 64-bit signed integer | `42`, `-7`, `0` |
| `float` | 64-bit floating point | `3.14`, `-0.5`, `1e10` |
| `string` | UTF-8 text string | `"Hello, World!"` |
| `bool` | Boolean values | `true`, `false` |
| `nil` | Null value | `nil` |

Type annotations are supported but optional — types are inferred when not specified.

```hunnu
let x: int = 42
let pi = 3.14       // inferred as float
```

---

## Variables

### Declaration

Use `let` to declare immutable variables:

```hunnu
let x = 10
let name = "Alice"
let is_active = true
```

Use `mut` to declare mutable variables:

```hunnu
let mut counter = 0
counter = counter + 1
```

### Assignment

```hunnu
let mut x = 10
x = 20
```

---

## Functions

### Declaration

```hunnu
fn add(a, b) {
    return a + b
}

fn greet(name) {
    print("Hello, " + name)
}
```

Functions without an explicit `return` return `nil`.

### Higher-Order Functions

Functions are first-class values:

```hunnu
fn apply(f, x) {
    return f(x)
}

fn double(n) {
    return n * 2
}

fn main() {
    print(apply(double, 5))  // 10
}
```

---

## Control Flow

### If/Else

```hunnu
if x > 10 {
    print("large")
} else if x > 5 {
    print("medium")
} else {
    print("small")
}
```

### While Loop

```hunnu
let i = 0
while i < 5 {
    print(i)
    i = i + 1
}
```

### For Loop

```hunnu
for let i = 0; i < 5; i = i + 1 {
    print(i)
}
```

### Break and Continue

```hunnu
while true {
    if x > 10 { break }
    x = x + 1
}

for let i = 0; i < 10; i = i + 1 {
    if i % 2 == 0 { continue }
    print(i)
}
```

---

## Expressions

### Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3` -> `8` |
| `-` | Subtraction | `5 - 3` -> `2` |
| `*` | Multiplication | `5 * 3` -> `15` |
| `/` | Division | `6 / 3` -> `2` |
| `%` | Modulo | `7 % 3` -> `1` |

### Comparison Operators

| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `<=` | Less than or equal |
| `>` | Greater than |
| `>=` | Greater than or equal |

### Logical Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `and` | Logical AND | `x > 0 and x < 10` |
| `or` | Logical OR | `x < 0 or x > 10` |
| `not` | Logical NOT | `not x == 0` |

### Operator Precedence

From highest to lowest:
1. `not` (unary), `-` (unary negate)
2. `*`, `/`, `%`
3. `+`, `-`
4. `<`, `<=`, `>`, `>=`
5. `==`, `!=`
6. `and`
7. `or`

### Literals

```hunnu
42         // integer
3.14       // float
"Hello"    // string
true       // boolean
nil        // null
```

### Grouping

```hunnu
let x = (a + b) * c
```

---

## Built-in Functions

### print

Prints a value to stdout:

```hunnu
print("Hello, World!")
print(42)
print(true)
```

### Math

```hunnu
import std.math
print(std.math.sqrt(16))
print(std.math.abs(-5))
```

---

## Arrays

Arrays are ordered, indexable collections:

```hunnu
let arr = [1, 2, 3]
print(arr[0])  // 1
print(arr[1])  // 2

let mut empty = []
empty[0] = 42
```

Standard library provides utilities:

```hunnu
import std.array
let a = [10, 20, 30]
print(std.array.first(a))      // 10
print(std.array.last(a))       // 30
print(std.array.len(a))        // 3

for let (i, v) in std.array.enumerate(a) {
    print(i)
    print(v)
}
```

---

## Structs

Structs (records) are defined with `type`:

```hunnu
type Point = { x, y }

fn Point.new(x_val, y_val) {
    return Point(x: x_val, y: y_val)
}

fn Point.length(self) {
    return self.x * self.x + self.y * self.y
}

fn main() {
    let p = Point.new(3, 4)
    print(p.length())  // 25
}
```

Fields can have type annotations:

```hunnu
type User = { name: string, age: int }
```

---

## Classes and OOP

### Class Declaration

```hunnu
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
```

The `pub` keyword makes fields publicly accessible. Without `pub`, fields are private.

### Inheritance

```hunnu
class Animal {
    pub name: str
    fn new(self, name) {
        self.name = name
    }
    fn speak(self) {
        print("...")
    }
}

class Dog : Animal {
    fn speak(self) {
        print("Woof!")
    }
}

fn main() {
    let d = new Dog("Rex")
    d.speak()  // Woof!
}
```

---

## Traits and Impl

### Trait Declaration

```hunnu
trait Area {
    fn area(self)
    fn describe(self)
}
```

### Implementation

```hunnu
trait Area {
    fn area(self)
}

impl Area for Circle {
    fn area(self) {
        return 3 * self.radius * self.radius
    }
}

fn main() {
    let c = new Circle(5)
    print(c.area())
}
```

---

## Match Expressions

### Basic Match

```hunnu
fn main() {
    let x = 2
    match x {
        1 -> print("one"),
        2 -> print("two"),
        _ -> print("other")
    }
}
```

### Range Patterns

```hunnu
fn main() {
    let x = 5
    match x {
        1..3 -> print("low"),
        4..6 -> print("medium"),
        _ -> print("high")
    }
}
```

### Array Destructuring

```hunnu
fn main() {
    let arr = [1, 2, 3]
    match arr {
        [a, b] -> print("two elements"),
        [a, b, c] -> print("three elements"),
        [a, ..rest] -> print("starts with " + a)
    }
}
```

### Option Matching

```hunnu
fn main() {
    let val = Some(42)
    match val {
        Option::Some(v) -> print(v),
        Option::None -> print("none")
    }
}
```

### Result Matching

```hunnu
fn main() {
    let val = Ok("success")
    match val {
        Result::Ok(v) -> print(v),
        Result::Err(e) -> print("error")
    }
}
```

---

## Error Handling

### Try/Catch

```hunnu
fn main() {
    try {
        let x = 10 / 0
    } catch {
        print("Caught an error!")
    }
}
```

With error variable:

```hunnu
fn main() {
    try {
        let x = risky_operation()
    } catch err {
        print("Error: " + err)
    }
}
```

With finally:

```hunnu
fn main() {
    try {
        // code that may fail
    } catch {
        // handle error
    } finally {
        // cleanup
    }
}
```

### Option/Result Types

```hunnu
let a = Some(42)
let b = None()
let c = Ok("success")
let d = Err("failure")

if is_some(a) { print("has value") }
if is_ok(c)   { print("ok") }
if is_err(d)  { print("error") }
```

---

## Modules and Imports

### Importing Modules

```hunnu
import std.math
import std.io
import std.array
import std.string
import std.fs
import std.time

fn main() {
    print(std.math.sqrt(16))
    print(std.io.read_file("test.txt"))
}
```

### Standard Library Modules

| Module | Contents |
|--------|----------|
| `std.libc` | C library FFI bindings |
| `std.math` | Math functions (sqrt, abs, pow, etc.) |
| `std.io` | I/O functions |
| `std.array` | Array utilities (first, last, len, enumerate) |
| `std.string` | String utilities (len, upcase, include, etc.) |
| `std.fs` | Filesystem utilities |
| `std.time` | Time functions |

---

## FFI (Foreign Function Interface)

Call C functions directly:

```hunnu
extern fn puts(s) -> int from "libc.so.6"
extern fn pow(base, exp) -> float from "libm.so.6"

fn main() {
    puts("Hello from C!")

    let result = pow(2.0, 3.0)
    print(result)  // 8.0
}
```

---

## Pointers and Unsafe

```hunnu
let x = 42
let p = &x     // take address
print(*p)      // dereference
```

Unsafe blocks for low-level operations:

```hunnu
unsafe {
    let p = &x
    *p = 99
}
```

---

## Enums

```hunnu
enum Color {
    Red,
    Green,
    Blue
}

fn main() {
    let c = Color.Red
}
```

---

## Internationalization

Hunnu supports Mongolian Cyrillic keywords alongside English.

### Running with Mongolian

```bash
./build/hunnu --lang mn hello.hn
# or via environment variable
HUNNU_LANG=mn ./build/hunnu hello.hn
```

### Example

```hunnu
функц main() {
    хувьсагч тоо = 10
    хэрвээ тоо > 5 {
        хэвлэх("5-аас их")
    }
}
```

### Rust Compiler Support

The Rust frontend (`compiler-rust/`) also recognizes Mongolian keywords via the
same bilingual lexer approach. Build with:

```bash
cd compiler-rust && cargo build
```

---

## AOT Compilation

Compile Hunnu source to a standalone native binary:

```bash
./build/hunnu compile hello.hn -o hello
./hello
```

The AOT backend transpiles Hunnu to C and compiles with gcc, producing
optimized native executables with no runtime dependency.

---

## Command Line Interface

### Building

```bash
mkdir build && cd build
cmake ..
make
```

### Commands

| Command | Description |
|---------|-------------|
| `hunnu run <file>` | Run a Hunnu program |
| `hunnu <file>` | Run (shorthand) |
| `hunnu tokens <file>` | Show token stream |
| `hunnu ast <file>` | Show AST |
| `hunnu compile <file> -o <output>` | AOT compile to native binary |

### Options

| Flag | Description |
|------|-------------|
| `--lang mn` | Use Mongolian keywords and error messages |
| `-v, --version` | Show version |
| `-h, --help` | Show help |

### Debugging

```bash
./build/hunnu tokens examples/main.hn
./build/hunnu ast examples/main.hn
```

---

## Grammar Reference

```
program       := declaration*

declaration   := let_decl | fn_decl | import_decl | extern_decl |
                 type_decl | class_decl | trait_decl | impl_decl |
                 enum_decl | statement

let_decl      := "let" IDENT (":" IDENT)? "=" expression
let_mut_decl  := "let" "mut" IDENT "=" expression

fn_decl       := "fn" IDENT "(" params? ")" block
params        := IDENT ("," IDENT)*

import_decl   := "import" IDENT ("." IDENT)*
extern_decl   := "extern" "fn" IDENT "(" params? ")" ("->" IDENT)? "from" STRING

type_decl     := "type" IDENT "=" "{" fields "}"
fields        := IDENT (":" IDENT)? ("," IDENT (":" IDENT)?)*

class_decl    := "class" IDENT (":" IDENT)? "{" class_body "}"
class_body    := (pub_field | method)*
pub_field     := "pub" IDENT ":" IDENT
method        := "fn" IDENT "(" "self" ("," params)? ")" block

trait_decl    := "trait" IDENT "{" trait_methods "}"
trait_methods := "fn" IDENT "(" "self" ")"
impl_decl     := "impl" IDENT "for" IDENT "{" impl_methods "}"
impl_methods  := "fn" IDENT "(" "self" ("," params)? ")" block

enum_decl     := "enum" IDENT "{" enum_variants "}"
enum_variants := IDENT ("," IDENT)*

statement     := if_stmt | while_stmt | for_stmt | return_stmt |
                 print_stmt | try_stmt | match_stmt |
                 block | expression_stmt | unsafe_stmt

if_stmt       := "if" expression block ("else" "if" expression block)?
                 ("else" block)?
while_stmt    := "while" expression block
for_stmt      := "for" (let_decl | expression) ";" expression ";" expression block
return_stmt   := "return" expression?
print_stmt    := "print" "(" expression ")"
try_stmt      := "try" block ("catch" IDENT? block)? ("finally" block)?
match_stmt    := "match" expression "{" match_arms "}"
match_arms    := pattern "->" expression ("," pattern "->" expression)*
unsafe_stmt   := "unsafe" block
block         := "{" declaration* "}"

pattern       := literal | IDENT | "_" | range_pattern | array_pattern |
                 "Option" "::" "Some" "(" IDENT ")" |
                 "Option" "::" "None" |
                 "Result" "::" "Ok" "(" IDENT ")" |
                 "Result" "::" "Err" "(" IDENT ")"
range_pattern := expression ".." expression
array_pattern := "[" pattern ("," pattern)* ("," ".." IDENT)? "]"

expression    := assignment
assignment    := primary "=" assignment | logical_or
logical_or    := logical_and ("or" logical_and)*
logical_and   := equality ("and" equality)*
equality      := comparison (("==" | "!=") comparison)*
comparison    := term (("<" | "<=" | ">" | ">=") term)*
term          := factor (("+" | "-") factor)*
factor        := unary (("*" | "/" | "%") unary)*
unary         := ("-" | "not") unary | postfix
postfix       := primary ("[" expression "]" | "." IDENT | "(" args? ")")*
primary       := INT | FLOAT | STRING | "true" | "false" | "nil" |
                 IDENT | "(" expression ")" | "[" elements? "]" |
                 "&" IDENT | "*" expression
elements      := expression ("," expression)*
args          := expression ("," expression)*
```
