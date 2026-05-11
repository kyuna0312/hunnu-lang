# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
# First time
mkdir -p build && cd build && cmake .. && make

# Rebuild from root
cd build && make

# Run a program
./build/hunnu run examples/main.hn
./build/hunnu run examples/main.hn --vm-rust  # Run with Rust VM

# Run with Mongolian
./build/hunnu --lang mn run examples/main.hn

# AOT compile
./build/hunnu compile examples/main.hn -o output && ./output

# Clean rebuild
rm -rf build/* && cd build && cmake .. && make
```

## Testing

```bash
# Run the test suite
./run_tests.sh
```

Tests are in `examples/test_*.hn` files. The test runner compiles and runs each one, comparing output against expected values.

## Architecture

Pipeline: `.hn` source -> Lexer -> tokens -> Parser -> AST -> Interpreter/VM -> output

All source under `compiler/`:
- `lexer/token.h` - `TokenType` enum, `Token` struct
- `lexer/lexer.c` - tokenizer; keywords registered in parallel `keyword_names[]` / `keyword_types[]` arrays (both EN + MN)
- `parser/parser.c` - recursive descent; each `parser_parse_*` function handles one grammar level
- `ast/ast.h` - `ASTNodeType` enum + tagged union `ASTNode` struct; one factory function per node type
- `interpreter/interpreter.c` - tree-walk interpreter; `interpreter_evaluate()` returns `Value`, `interpreter_execute_statement()` is void
- `i18n/i18n.c` - English/Mongolian bilingual error messages and keyword mapping

The `Value` struct (`compiler/value.h`) uses a `type` enum:
```
VALUE_INT, VALUE_FLOAT, VALUE_STRING, VALUE_BOOL,
VALUE_NONE, VALUE_ARRAY, VALUE_STRUCT, VALUE_POINTER
```

Scope is managed via `compiler/scope.c` (`Scope` struct with `scope_define()`, `scope_lookup()`, scope chaining).

## Adding Features

### New token
1. `compiler/lexer/token.h` - add to `TokenType` enum
2. `compiler/lexer/lexer.c` - add to `keyword_names[]` + `keyword_types[]` (keywords), or handle in main switch (symbols)
3. `compiler/i18n/i18n.c` - add Mongolian translation

### New AST node
1. `compiler/ast/ast.h` - add to `ASTNodeType`, add struct to the `union`, declare factory function
2. `compiler/ast/ast.c` - implement factory + `ast_free` case + `ast_print` case
3. `compiler/parser/parser.c` - build the node
4. `compiler/interpreter/interpreter.c` - evaluate/execute the node
5. `compiler/transpile/transpile.c` - emit C code for AOT mode (optional)

### OOP Features (Month 4)
- Classes: `class Name { pub field, fn new(self) { ... }, fn method(self) { ... } }`
- Inheritance: `class Child : Parent { ... }` (parent_name field in class_decl)
- Traits: `trait Name { fn method(self); }` (method signatures only)
- Impl: `impl Trait for Type { fn method(self) { ... } }` (registers TypeName.methodName)
- Method dispatch: looks up `TypeName.methodName` in user_fns array; walks inheritance chain
- Encapsulation: `pub` keyword; warnings on private field access
- AOT: top-level classes only; runtime dispatch via _reg_method / hunnu_method_call

### i18n
- Both English AND Mongolian keywords always work in source code
- `--lang mn` flag or `HUNNU_LANG=mn` env var for Mongolian error messages
- Keywords are embedded in lexer's keyword_names[] array as pairs (EN at even indices, MN at odd)
- Add both forms to `keyword_names[]` + `keyword_types[]` when adding a new keyword

## Code Style

- 4-space indent, 100-char line limit, opening brace on same line
- `static` for file-internal functions; `const` wherever possible
- Includes order: local `.h` first, then stdlib headers
- Naming: `snake_case` for functions/variables, `UPPER_SNAKE_CASE` for enums/macros, `PascalCase` for struct type names
- Error output via `fprintf(stderr, ...)`, never silently ignored
- Always initialize pointers to `NULL`; check before dereference

## Known Gotchas

- Assignment (`AST_ASSIGN`) must live in `interpreter_evaluate()` (returns `Value`), **not** in `interpreter_execute_statement()` (void).
- `while` loops use no parentheses in Hunnu syntax: `while x > 0 { ... }` (not `while(x > 0)`).
- Newlines are tokens (`TOKEN_NEWLINE`); `parser_skip_newlines()` must be called at statement boundaries.
- Module imports (`import std.math`) are converted to file paths (`stdlib/math.hn`) in `cli/main.c`.
- Classes defined inside functions cannot be AOT-compiled (C constraint); only top-level classes work.
