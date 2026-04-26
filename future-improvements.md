# Future Improvements

> Potential enhancements for Hunnu beyond the MVP.

## Language Features

### High Priority
- **Bytecode compiler + VM** ✅ (DONE)
- **Modules / import system** — split code across files
- **Standard library** — built-in functions for common tasks

### Medium Priority
- **Structs / records** — grouping related data:
  ```hunnu
  type Point = { x: int, y: int }
  let p = Point { x: 5, y: 10 }
  ```

- **Pattern matching** — destructuring and case analysis:
  ```hunnu
  match value {
      [] -> "empty"
      [head, ...rest] -> "first: " + str(head)
  }
  ```

- **Algebraic Data Types (ADTs)** — sum types:
  ```hunnu
  type Maybe[T] = Just(T) | Nothing
  type List[T] = Cons(T, List[T]) | Nil
  ```

### Lower Priority
- **Pipe operator** — function chaining:
  ```hunnu
  x |> double |> add(5) |> str
  ```

- **Lazy evaluation** — deferred computation:
  ```hunnu
  let lazy_val = lazy expensive_compute()
  ```

- **Structural types** — duck typing:
  ```hunnu
  let point = { x: 5, y: 10 }  // inferred as { x: int, y: int }
  ```

- **Gradual typing** — optional type annotations:
  ```hunnu
  let x: int = 5
  fn add(a: int, b: int) -> int { a + b }
  ```

- **Protocols/Traits** — interface definitions:
  ```hunnu
  protocol Printable {
      fn format(self) -> string
  }
  ```

## Implementation

### High Priority
- **Optimization** — performance improvements
- **Error messages** — better diagnostics with line numbers
- **Debugger** — stepping through code

### Medium Priority
- **REPL** — interactive console
- **Package manager** — dependency management
- **IDE integration** — LSP server

### Lower Priority
- **Self-hosting** — implement compiler in Hunnu itself
- **JIT compilation** — just-in-time code generation
- **AOT compilation** — ahead-of-time binary output

## Infrastructure

### Testing
- Unit tests for compiler phases
- Integration tests for language features
- Benchmark suite
- Fuzzing for edge cases

### Documentation
- Language specification
- API documentation
- Tutorial for beginners
- Cookbook with recipes

### Community
- Standard library contributions
- Tooling ecosystem
- Package registry