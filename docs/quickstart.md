# Hunnu Quickstart Guide

Welcome to Hunnu! This guide will get you up and running with the language.

## Installation

### Prerequisites
- C compiler (gcc/clang)
- CMake >= 3.10
- make

### Build from Source

```bash
git clone https://github.com/hunnu-labs/hunnu-lang.git
cd hunnu-lang
mkdir -p build && cd build
cmake ..
make
```

The `hunnu` binary will be at `build/hunnu`.

## Hello World

Create a file `hello.hn`:

```hunnu
fn main() {
    print("Hello, World!")
}
```

Run it:

```bash
./build/hunnu run hello.hn
```

## Language Basics

### Variables

```hunnu
fn main() {
    let x = 10
    let name = "Hunnu"
    let pi = 3.14
    print(x)
    print(name)
}
```

Variables are immutable by default. Use `mut` for mutable bindings:

```hunnu
fn main() {
    let mut counter = 0
    counter = counter + 1
    print(counter)
}
```

### Functions

```hunnu
fn add(a, b) {
    return a + b
}

fn main() {
    let result = add(3, 4)
    print(result)
}
```

### Control Flow

```hunnu
fn main() {
    let x = 10

    if x > 5 {
        print("greater")
    } else {
        print("less or equal")
    }

    let i = 0
    while i < 5 {
        print(i)
        i = i + 1
    }

    for let j = 0; j < 3; j = j + 1 {
        print(j)
    }
}
```

### Arrays

```hunnu
fn main() {
    let arr = [1, 2, 3]
    print(arr[0])
    print(arr[1])
    print(arr[2])
}
```

### Structs

```hunnu
type Point = { x, y }

fn Point.new(x_val, y_val) {
    return Point(x: x_val, y: y_val)
}

fn main() {
    let p = Point.new(10, 20)
    print(p.x)
    print(p.y)
}
```

## Object-Oriented Programming

### Classes

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
    print(p.length())
}
```

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
    d.speak()
}
```

### Traits

```hunnu
trait Area {
    fn area(self)
}

impl Area for Circle {
    fn area(self) {
        return 3 * self.radius * self.radius
    }
}
```

## Match Expressions

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

## Error Handling

```hunnu
fn main() {
    try {
        let x = 10 / 0
    } catch {
        print("Caught an error!")
    }
}
```

## Modules and Imports

```hunnu
import std.math
import std.io

fn main() {
    print(std.math.sqrt(16))
}
```

## FFI (Foreign Function Interface)

```hunnu
extern fn puts(s) -> int from "libc.so.6"

fn main() {
    puts("Hello from C!")
}
```

## Mongolian Keywords

Hunnu supports Mongolian Cyrillic keywords:

```hunnu
функц main() {
    хувьсагч тоо = 10
    хэрвээ тоо > 5 {
        хэвлэх("5-аас их")
    }
}
```

Run with Mongolian locale:

```bash
./build/hunnu --lang mn hello.hn
```

## AOT Compilation

Compile to a native binary:

```bash
./build/hunnu compile hello.hn -o hello
./hello
```

## Next Steps

- Read the full [Language Specification](language-spec.md)
- Explore examples in the `examples/` directory
- Check the standard library in `stdlib/`
