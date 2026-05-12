# AGENTS.md - Hunnu Language Development Guidelines

This file provides guidance for agentic coding agents working on hunnu-lang.

## Project Overview

hunnu-lang is a lightweight, expression-oriented programming language written in C (interpreter) and Rust (AOT compiler).
The project uses CMake for building and has a compiler/interpreter architecture.

**Current Phase:** v1.0.0 (Эрдэнэ) — Month 6 Complete (Self-Hosting, Package Manager, Release)

---

## Build Commands

CMake is used as the build system.

### Main Commands

```bash
# Create build directory (first time only)
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build the project
make

# Or build from root:
cd build && make
```

### Running

```bash
# Run a Hunnu program
./build/hunnu run examples/main.hn

# Or shorter:
./build/hunnu examples/main.hn
```

---

## Code Style Guidelines

### General Principles

- Write clean, readable, idiomatic C code
- Keep functions small and focused (single responsibility)
- Use meaningful variable and function names
- Avoid magic numbers - use constants
- Comment "why", not "what"

### Formatting

- Use 4 spaces for indentation (no tabs)
- Max line length: 100 characters
- Opening brace on same line as function/if/while
- Use `const` whenever possible
- Prefer `static` for internal functions

Example:
```c
static int calculate_value(int a, int b) {
    return a + b;
}
```

### Includes

- Group includes in this order:
  1. Project header (local .h)
  2. Standard C library (<stdio.h>, <stdlib.h>, etc.)
  3. System headers

Example:
```c
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
```

### Naming Conventions

| Type | Convention | Example |
|------|-----------|---------|
| Variables | snake_case | `current_token` |
| Constants | UPPER_SNAKE_CASE | `MAX_TOKEN_LENGTH` |
| Functions | snake_case | `parse_expression` |
| Types/Structs | snake_case (PascalCase for display) | `Lexer` |
| Enums (values) | UPPER_SNAKE_CASE | `TOKEN_LET` |
| Files | snake_case | `lexer.c` / `lexer.h` |
| Macros | UPPER_SNAKE_CASE | `#define MAX_DEPTH 100` |

### Types

- Use fixed-width integers from `<stdint.h>`: `int32_t`, `int64_t`, `uint32_t`
- Use `size_t` for sizes and indices
- Use `int` for boolean return types (0 = false, non-zero = true)
- Always initialize pointers to `NULL`
- Check for `NULL` before dereferencing

### Error Handling

- Return error codes (0 for success, negative for errors)
- Use `fprintf(stderr, ...)` for error messages
- Never silently ignore return values
- Clean up resources on error (free memory, close files)

---

## Project Architecture

### Directory Structure

```
hunnu-lang/
├── compiler/
│   ├── ast/          # Abstract syntax tree definitions
│   ├── interpreter/  # Runtime execution
│   ├── lexer/       # Tokenization (lexer.c, token.h)
│   └── parser/       # Syntax analysis
├── cli/              # Command-line interface
├── stdlib/            # Standard library modules
│   ├── libc.hn       # C library FFI bindings
│   ├── math.hn       # Math functions
│   ├── io.hn         # I/O functions
│   ├── array.hn      # Array utilities
│   ├── string.hn     # String utilities
│   ├── fs.hn         # Filesystem functions
│   └── time.hn       # Time functions
├── vm-rust/           # Rust VM implementation
├── bindings/          # Language bindings
│   └── python/       # Python bindings (PyO3)
├── examples/          # Sample .hn programs
├── build/            # Build output (gitignored)
└── CMakeLists.txt   # Build configuration
```

### Key Files

- `compiler/lexer/token.h` - Token type definitions (enum TokenType)
- `compiler/lexer/lexer.c` - Lexical analyzer implementation
- `compiler/parser/parser.c` - Parser implementation (dispatch core)
- `compiler/parser/parse_decl.c` - Declaration parsers
- `compiler/parser/parse_stmt.c` - Statement parsers
- `compiler/parser/parse_expr.c` - Expression parsers
- `compiler/ast/ast.h` - AST node definitions
- `compiler/interpreter/interpreter.c` - Lifecycle + state helpers
- `compiler/interpreter/eval.c` - Expression evaluation
- `compiler/interpreter/exec.c` - Statement execution
- `compiler/interpreter/call.c` - Function call dispatch

### Data Flow

```
Source Code (.hn)
    │
    ▼
Lexer (tokenizer)
    │ tokens
    ▼
Parser (AST builder)
    │ AST nodes
    ▼
Interpreter (lifecycle/interpreter.c)
    ├── eval.c   → expression evaluation (interpreter_evaluate)
    ├── exec.c   → statement execution (interpreter_execute_statement)
    └── call.c   → function call dispatch (interpreter_call_user_fn)
    │
    ▼
Output
```

---

## Adding New Features

### Adding New Tokens

1. Add `TOKEN_NEW_TOKEN` to `compiler/lexer/token.h` enum
2. Add keyword in `compiler/lexer/lexer.c` `keyword_names` array
3. Add corresponding type in `keyword_types` array
4. Update `lexer_check_keyword()` if needed

### Adding New AST Nodes

1. Add `AST_NEW_NODE` to `compiler/ast/ast.h` enum
2. Create struct in `ast.h` for the node
3. Implement creation/destruction functions in `ast.c`
4. Update parser to build the node
5. Update interpreter to execute the node

### Adding Keywords

Keywords are handled in `lexer.c`:
- Add keyword string to `keyword_names` array
- Add corresponding `TokenType` to `keyword_types` array

---

## Testing

### Automated Testing

```bash
# Run all tests via ctest
cd build && make && ctest

# Run just the C unit tests
./build/tests/hunnu_tests

# Run integration tests on example files
./run_tests.sh

# Run specific test via ctest
cd build && ctest -R hunnu_c_unit_tests
```

### C Unit Tests (50 tests in tests/)

The C unit test framework uses [minunit](https://github.com/siu/minunit), a minimal header-only test framework.

| Suite | File | Tests | What it covers |
|-------|------|-------|----------------|
| Value | `tests/test_value.c` | 12 | create/free/copy values, type checks |
| Scope | `tests/test_scope.c` | 6 | define/lookup/nested/shadowing |
| Lexer | `tests/test_lexer.c` | 10 | tokens: int/float/string/keywords/operators |
| Parser | `tests/test_parser.c` | 11 | AST nodes: decls/expressions/statements/errors |
| Interpreter | `tests/test_interpreter.c` | 11 | runtime: arithmetic/fn calls/loops/scopes |

**When adding features, add tests to the corresponding test suite file.**

### Test Program Example

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

---

## IDE Recommendations

- Use VS Code or Cursor
- Install C/C++ extension for syntax highlighting
- Use `.vscode/c_cpp_properties.json` for include paths:

```json
{
    "configurations": [
        {
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/compiler"
            ]
        }
    ]
}
```

---

## Common Issues

### Build Failures

If build fails, clean and rebuild:
```bash
rm -rf build/*
cd build && cmake .. && make
```

### Segmentation Faults

- Use a debugger: `gdb ./build/hunnu`
- Check for uninitialized pointers
- Verify NULL checks before dereferencing

---

## Project Structure

### Current (Month 6 - v1.0 Release)

```
hunnu-lang/
├── compiler/
│   ├── ast/          # Abstract syntax tree definitions
│   ├── interpreter/  # Runtime execution (C tree-walk)
│   │   ├── builtins.c/h  # Shared builtin implementations
│   │   ├── interpreter.c # Lifecycle + state helpers
│   │   ├── eval.c        # Expression evaluation
│   │   ├── exec.c        # Statement execution
│   │   └── call.c        # Function call dispatch
│   ├── lexer/       # Tokenization (lexer.c, token.h)
│   ├── parser/       # Syntax analysis
│   └── transpile/    # C transpiler backend (AOT via gcc)
├── compiler-rust/     # Rust compiler frontend (Month 3)
│   ├── src/
│   │   ├── lib.rs     # Main entry point
│   │   ├── lexer.rs   # Rust lexer
│   │   ├── parser.rs  # Rust parser
│   │   ├── ast.rs     # Rust AST definitions
│   │   └── codegen.rs # LLVM IR codegen (skeleton)
│   └── Cargo.toml
├── vm-rust/           # Rust VM (Month 1)
├── cli/              # Command-line interface (incl. package manager)
├── stdlib/            # Standard library modules (all v1 complete)
├── self/              # Self-hosting compiler (Hunnu in Hunnu)
│   ├── token.hn       # Token definitions
│   └── lexer.hn       # Lexer implementation
├── benchmarks/        # hunnu-benchmark submodule
├── bindings/
│   └── python/       # Python bindings (PyO3)
├── examples/         # Sample .hn programs
├── build/            # Build output (gitignored)
├── .github/workflows/ # CI/CD pipelines (build, test, release)
└── CMakeLists.txt   # Build configuration
```

## Notes for Agents

- Always build and test after changes
- Keep changes small and focused
- Update examples to demonstrate new features
- Update this file when project setup changes
- **Language rule: Do NOT use Chinese characters in code, comments, or documentation**

## Language Rule

**Agents must NOT write Chinese characters (中文) anywhere in the codebase including:**
- Source code (C, Rust, Python, etc.)
- Comments and documentation
- Commit messages
- Markdown files (README.md, AGENTS.md, plan.md, etc.)
- Test files and scripts

**All code, comments, and documentation must be written in English only.**

---

## Hunnu OOP Language Reference

### Structs with methods
```hunnu
type Point = { x, y }
fn Point.new(x_val, y_val) {
    return Point(x: x_val, y: y_val)
}
fn Point.length(self) {
    return self.x * self.x + self.y * self.y
}
```

### Classes with constructor
```hunnu
class Point {
    pub x: int
    pub y: int
    fn new(self, x, y) { self.x = x; self.y = y }
    fn length(self) { return self.x * self.x + self.y * self.y }
}
let p = new Point(3, 4)
print(p.length())
```

### Inheritance (`class Child : Parent`)
```hunnu
class Dog : Animal {
    pub breed: str
    fn new(self, name, breed) { self.name = name; self.breed = breed }
    fn speak(self) { print("Woof!") }
}
```

### Traits and Impls
```hunnu
trait Area {
    fn area(self)
    fn describe(self)
}
impl Area for Circle {
    fn area(self) { return 3 * self.radius * self.radius }
    fn describe(self) { print("Circle") }
}
```

### AOT Compilation
Classes must be at top-level for AOT mode:
```bash
hunnu compile file.hn -o output
```