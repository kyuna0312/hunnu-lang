# AGENTS.md - Hunnu Language Development Guidelines

This file provides guidance for agentic coding agents working on hunnu-lang.

## Project Overview

hunnu-lang is a lightweight, expression-oriented programming language written in C.
The project uses CMake for building and has a compiler/interpreter architecture.

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
├── examples/          # Sample .hn programs
├── build/            # Build output (gitignored)
└── CMakeLists.txt   # Build configuration
```

### Key Files

- `compiler/lexer/token.h` - Token type definitions (enum TokenType)
- `compiler/lexer/lexer.c` - Lexical analyzer implementation
- `compiler/parser/parser.c` - Parser implementation
- `compiler/ast/ast.h` - AST node definitions
- `compiler/interpreter/interpreter.c` - Runtime interpreter

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
Interpreter
    │ execution
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

### Manual Testing

```bash
# Build the project
cd build && make

# Run an example
./build/hunnu examples/main.hn

# Create test files in examples/ directory
```

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