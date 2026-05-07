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

# Clean rebuild
rm -rf build/* && cd build && cmake .. && make
```

No automated test suite — testing is manual via example `.hn` files in `examples/`.

## Architecture

Pipeline: `.hn` source → Lexer → tokens → Parser → AST → Interpreter/VM → output

All source under `compiler/`:
- `lexer/token.h` — `TokenType` enum, `Token` struct
- `lexer/lexer.c` — tokenizer; keywords registered in parallel `keyword_names[]` / `keyword_types[]` arrays
- `parser/parser.c` — recursive descent; each `parser_parse_*` function handles one grammar level
- `ast/ast.h` — `ASTNodeType` enum + tagged union `ASTNode` struct; one factory function per node type
- `interpreter/interpreter.c` — tree-walk interpreter; `interpreter_evaluate()` returns `Value`, `interpreter_execute_statement()` is void

The `Value` struct (`interpreter.h`) uses a `type` enum (`VALUE_INT`, `VALUE_STRING`, `VALUE_FLOAT`, `VALUE_BOOL`, `VALUE_NONE`) plus a `has_value` flag for uninitialized state.

Environment (variable scope) is managed inside `interpreter.c` via `interpreter_define()`, which updates existing variables instead of shadowing them.

## Adding Features

### New token
1. `compiler/lexer/token.h` — add to `TokenType` enum
2. `compiler/lexer/lexer.c` — add to `keyword_names[]` + `keyword_types[]` (keywords), or handle in the main `switch` (symbols)

### New AST node
1. `compiler/ast/ast.h` — add to `ASTNodeType`, add struct to the `union`, declare factory function
2. `compiler/ast/ast.c` — implement factory + `ast_free` case + `ast_print` case
3. `compiler/parser/parser.c` — build the node
4. `compiler/interpreter/interpreter.c` — evaluate/execute the node

### Try/Catch (Month 2 feature)
- Added `TOKEN_TRY`, `TOKEN_CATCH`, `TOKEN_FINALLY` to `compiler/lexer/token.h`
- Added `AST_TRY_STMT` to `compiler/ast/ast.h`
- Parser supports `try { } catch { }` syntax
- Interpreter executes try/catch/finally blocks

### Module System (Month 2 feature)
- Import path resolution supports both file paths and module paths
- `import "stdlib/math.hn"` — file import
- `import std.math` — module import (converts to `stdlib/math.hn`)

## Code Style

- 4-space indent, 100-char line limit, opening brace on same line
- `static` for file-internal functions; `const` wherever possible
- Includes order: local `.h` first, then stdlib headers
- Naming: `snake_case` for functions/variables, `UPPER_SNAKE_CASE` for enums/macros, `PascalCase` for struct type names
- Error output via `fprintf(stderr, ...)`, never silently ignored
- Always initialize pointers to `NULL`; check before dereference

## Known Gotchas

- Assignment (`AST_ASSIGN`) must live in `interpreter_evaluate()` (returns `Value`), **not** in `interpreter_execute_statement()` (void) — a previous bug caused by the wrong placement.
- `while` loops use no parentheses in Hunnu syntax: `while x > 0 { ... }` (not `while(x > 0)`).
- Newlines are tokens (`TOKEN_NEWLINE`); `parser_skip_newlines()` must be called at statement boundaries to avoid parse failures.
- Module imports (`import std.math`) are converted to file paths (`stdlib/math.hn`) in `cli/main.c` `resolve_import_path()`.
