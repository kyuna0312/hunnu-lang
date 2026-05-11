# Hunnu Language Documentation

> **DEPRECATION NOTICE:** This document is severely outdated (covers only basic features from Phase 1-2).
> It does NOT include: arrays, structs, pointers, FFI, try/catch, imports/modules, match expressions,
> classes, inheritance, traits/impl, OOP, bytecode VM, AOT compilation, i18n/Mongolian keywords,
> or any Month 2-4 features. See the [README](../README.md) and [examples](../examples/) for current usage,
> and [plan.md](../plan.md) for the development roadmap.
> A full rewrite is tracked in issue #48.

## Overview

Hunnu is a lightweight, expression-oriented programming language written in C. It features a clean syntax inspired by C-family languages with a focus on simplicity and readability.

## Table of Contents

1. [Lexical Structure](#lexical-structure)
2. [Types](#types)
3. [Variables](#variables)
4. [Functions](#functions)
5. [Control Flow](#control-flow)
6. [Expressions](#expressions)
7. [Built-in Functions](#built-in-functions)
8. [Command Line Interface](#command-line-interface)

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

```
let     fn      if      else     while    for
return  print   true    false    not      and     or
```

### Whitespace

Hunnu ignores whitespace (spaces, tabs, newlines) between tokens. Statements can be placed on the same line or split across multiple lines.

---

## Types

Hunnu has the following built-in types:

| Type | Description | Example |
|------|-------------|---------|
| `int` | 64-bit signed integer | `42`, `-7`, `0` |
| `string` | UTF-8 text string | `"Hello, World!"` |
| `bool` | Boolean values | `true`, `false` |

Type annotations are not required—types are inferred automatically.

---

## Variables

### Declaration

Use `let` to declare variables:

```hunnu
let x = 10
let name = "Alice"
let is_active = true
```

### Assignment

Reassign variables using `=`:

```hunnu
let x = 10
x = 20  // x is now 20
```

---

## Functions

### Declaration

Functions are declared with the `fn` keyword:

```hunnu
fn greet(name) {
    print("Hello, " + name)
}

fn add(a, b) {
    return a + b
}
```

### Parameters

Functions can take multiple parameters separated by commas:

```hunnu
fn calculate(x, y, z) {
    return x + y + z
}
```

### Return Values

Use `return` to return a value:

```hunnu
fn square(n) {
    return n * n
}
```

Functions without an explicit return statement return `nil`.

### Calling Functions

```hunnu
let result = add(5, 3)
print(result)  // prints: 8
```

---

## Control Flow

### If Statement

```hunnu
if condition {
    // code
}
```

With else branch:

```hunnu
if x > 10 {
    print("large")
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

The for loop has three parts: initializer, condition, and update:

```hunnu
for let i = 0; i < 5; i = i + 1 {
    print(i)
}
```

### Break and Continue

Use `break` to exit a loop early:

```hunnu
while true {
    if x > 10 {
        break
    }
    x = x + 1
}
```

Use `continue` to skip to the next iteration:

```hunnu
for let i = 0; i < 10; i = i + 1 {
    if i % 2 == 0 {
        continue
    }
    print(i)
}
```

---

## Expressions

### Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3` → `8` |
| `-` | Subtraction | `5 - 3` → `2` |
| `*` | Multiplication | `5 * 3` → `15` |
| `/` | Division | `6 / 3` → `2` |
| `%` | Modulo | `7 % 3` → `1` |

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
1. `not` (unary)
2. `*`, `/`, `%`
3. `+`, `-`
4. `<`, `<=`, `>`, `>=`
5. `==`, `!=`
6. `and`
7. `or`

### Literals

**Integer literals:**
```hunnu
42
-7
0
```

**String literals:**
```hunnu
"Hello"
"World"
```

**Boolean literals:**
```hunnu
true
false
```

### Grouping

Use parentheses to group expressions:

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

---

## Command Line Interface

### Building

```bash
mkdir build && cd build
cmake ..
make
```

### Running Programs

```bash
./build/hunnu run examples/main.hn
```

### Debugging Commands

**Show tokens:**
```bash
./build/hunnu tokens examples/main.hn
```

**Show AST:**
```bash
./build/hunnu ast examples/main.hn
```

### Options

| Flag | Description |
|------|-------------|
| `-v, --version` | Show version information |
| `-h, --help` | Show help message |

---

## Example Programs

### Hello World

```hunnu
fn main() {
    print("Hello, World!")
}
```

### Factorial

```hunnu
fn factorial(n) {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}

fn main() {
    let result = factorial(5)
    print(result)  // prints: 120
}
```

### FizzBuzz

```hunnu
fn main() {
    for let i = 1; i <= 15; i = i + 1 {
        if i % 15 == 0 {
            print("FizzBuzz")
        } else if i % 3 == 0 {
            print("Fizz")
        } else if i % 5 == 0 {
            print("Buzz")
        } else {
            print(i)
        }
    }
}
```

### Sum of Array

```hunnu
fn sum(arr, len) {
    let total = 0
    for let i = 0; i < len; i = i + 1 {
        total = total + arr[i]
    }
    return total
}
```

---

## Grammar Reference

```
program     := declaration*

declaration := let_decl | fn_decl | statement

let_decl    := "let" IDENT "=" expression ";"

fn_decl     := "fn" IDENT "(" params? ")" block
params      := IDENT ("," IDENT)*

statement   := if_stmt | while_stmt | for_stmt | return_stmt | print_stmt | block | expression_stmt

if_stmt     := "if" expression statement ("else" statement)?
while_stmt  := "while" "(" expression ")" statement
for_stmt    := "for" (let_decl | expression ";") expression ";" expression statement
return_stmt := "return" expression?
print_stmt  := "print" "(" expression ")"

block       := "{" declaration* "}"

expression  := assignment
assignment  := IDENT "=" assignment | equality
equality    := comparison (("==" | "!=") comparison)*
comparison  := term (("<" | "<=" | ">" | ">=") term)*
term        := factor (("+" | "-") factor)*
factor      := unary (("*" | "/" | "%") unary)*
unary       := ("-" | "not") unary | primary
primary     := INT | STRING | "true" | "false" | IDENT | "(" expression ")"

```

---

## Error Handling

Hunnu provides error messages with line and column numbers:

```
[2:5] Error: Expected expression
```

Common errors:
- Missing semicolons or parentheses
- Undefined variables
- Division by zero
- Type mismatches

---

## Roadmap

- [ ] Type annotations
- [ ] Arrays/lists
- [ ] Standard library
- [ ] Bytecode compilation
- [ ] REPL mode
- [ ] Modules/imports
- [ ] Closures
- [ ] Error handling with try/catch
- [ ] Classes and objects