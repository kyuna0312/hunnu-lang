# Hunnu Language — Development Plan

> A living document tracking the state, priorities, and vision for Hunnu.

---

## Current Language State

### Working Features
- Variables: `let x = 5`
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `>`, `<`, `>=`, `<=`, `==`, `!=`
- Boolean: `and`, `or`, `not`
- If/else statements with `else if` chains
- While loops: `while(condition) { body }`
- For loops: `for(init; condition; update) { body }`
- Functions: `fn name(param) { body }`
- Return statements: `return expression`
- Print: `print(value)`
- Variable reassignment: `x = new_value`
- Compound assignment: `x += 1`, `x -= 2`, `x *= 3`, `x /= 4`
- Arrays: `[1, 2, 3]` + indexing `arr[i]`
- String concatenation: `"Hello" + "World"`
- String escapes: `\n`, `\t`, `\\`, `\"`
- `len()` built-in function
- `input()` — read user input from stdin
- `to_str()`, `to_int()`, `to_float()` — type conversions
- First-class function calls (by name)
- **Scoped variables** — block-scoped with scope stack
- **break/continue** — loop control flow
- Proper variable scoping in blocks and functions
- **Floating-point numbers**: `3.14159`, `2.0`
- **null/nil literal**: `let x = null` or `let y = nil`
- **CLI**: `--debug` flag shows tokens and AST

### Missing / Broken
- (All Phase 1 items completed!)

---

## Priority Roadmap

### Phase 1: Foundation Fixes ✅

| Priority | Item | Files | Status |
|----------|------|-------|--------|
| 🔴 | Variable scoping (scope stack) | interpreter.c | ✅ DONE |
| 🔴 | break/continue execution | interpreter.c, token.h, parser.c | ✅ DONE |
| 🔴 | Array index bounds checking | interpreter.c | ✅ DONE |
| 🔴 | String memory safety (dangling pointers) | parser.c, interpreter.c | ✅ DONE |

### Phase 2: Core Language Features ✅

| Priority | Item | Files | Status |
|----------|------|-------|--------|
| 🟡 | Compound assignment: `+=`, `-=`, etc | parser.c | ✅ DONE |
| 🟡 | `else if` chains | parser.c | ✅ DONE |
| 🟡 | Floating-point numbers | lexer.c, token.h, interpreter.c | ✅ DONE |
| 🟡 | `null`/`nil` literal | lexer.c, token.h, ast.h | ✅ DONE |
| 🟢 | String escapes: `\n`, `\t`, `\\` | lexer.c | ✅ DONE |
| 🟢 | Multi-line strings | lexer.c | ⚠️ SKIPPED (strings already work across lines) |

### Phase 3: Standard Library & Dev Experience ✅

| Priority | Item | Files | Status |
|----------|------|-------|--------|
| 🟡 | `input()` — read stdin | interpreter.c, parser.c | ✅ DONE |
| 🟡 | `to_str()`, `to_int()`, `to_float()` conversions | interpreter.c, parser.c | ✅ DONE |
| 🟢 | `--debug` / `--ast` CLI flag | cli.c | ✅ DONE |
| 🟢 | Runtime errors with line numbers | interpreter.c | ⚠️ Already has (basic) |

### Phase 4: Advanced Features

| Priority | Item | Status |
|----------|------|--------|
| 🔵 | Bytecode compiler + VM | TODO |
| 🔵 | Modules / import system | TODO |
| 🔵 | Structs / records | TODO |
| 🔵 | Pattern matching | TODO |
| 🔵 | Algebraic Data Types (ADTs) | TODO |

---

## Long-Term Vision

These are aspirational features that define Hunnu's future direction.

### Self-Hosting (Bootstrap)
A language should be able to implement itself. Once Hunnu is mature:
- Rewrite compiler/interpreter in Hunnu
- Create trusted bootchain
- Enable metaprogramming and macros

### Polish Notation (Prefix Syntax)
Alternative syntax mode for metaprogramming:

```hunnu
+ 5 3                    # 5 + 3
* + 2 3 4                # (2 + 3) * 4
if > x 0 { + x 1 }       # if x > 0 { x + 1 }
```

Benefits:
- No operator precedence ambiguity
- Uniform syntax = simpler parser
- Code is data (homoiconic)
- Opens door to Lisp-style macros

### Functional Programming Features
Inspired by Elixir, Lean, and Haskell:

```hunnu
# Pattern matching
match x {
    [] -> "empty"
    [head, ...rest] -> "head: " + str(head)
}

# Pipe operator
x |> double |> add(5) |> str

# Guards
fib(n) where n > 1 -> fib(n-1) + fib(n-2)

# Lazy evaluation
let lazy_val = lazy expensive_compute()
```

### Flexible Type System
Balance compile-time safety with runtime flexibility:

```hunnu
# Gradual typing (optional annotations)
let x = 5              # inferred as int
let y: int = 5         # explicitly typed

# Structural types
let point = { x: 5, y: 10 }  # inferred as { x: int, y: int }

# Protocols/Traits
protocol Printable {
    fn format(self) -> string
}

# ADTs (sum types)
type Maybe[T] = Just(T) | Nothing
type List[T] = Cons(T, List[T]) | Nil

# Dependent types (stretch goal)
type Vec[T, n: int] = ...  # vector of length n
```

### Error Handling Model
Move toward explicit, pattern-matching-based errors:

```hunnu
# Result types
let result = parse_int("42")
match result {
    Ok(n) -> n * 2
    Err(e) -> handle_error(e)
}

# Supervision trees for fault-tolerant systems
```

---

## Implementation Order

```
Phase 1 (Foundation)      Phase 2 (Core)         Phase 3 (Lib/DX)        Phase 4 (Advanced)
─────────────────────      ──────────────         ────────────────        ──────────────────
┌─────────────────┐        ┌─────────────┐        ┌───────────────┐        ┌─────────────┐
│ Scoped envs     │        │ += -= *= /= │        │ input()       │        │ Bytecode VM │
│ break/continue  │──────▶│ else if     │───────▶│ str/int()     │───────▶│ Modules     │
│ Array bounds    │        │ Float nums  │        │ CLI debug flag│        │ Structs     │
│ String memory   │        │ null/nil    │        │ Line errors   │        │ Pattern mat │
└─────────────────┘        │ String esc  │        └───────────────┘        │ ADTs        │
                           └─────────────┘                                   └─────────────┘
                                                                                    │
                                                                                    ▼
                              Self-hosting ◀────────────── Polish notation ◀───────┤
                                                                                    │
                              Flexible types ◀────────────── Functional features ◀──┘
```

---

## Files Map

| Component | Files | Purpose |
|-----------|-------|---------|
| Lexer | `compiler/lexer/lexer.c`, `compiler/lexer/token.h` | Tokenization |
| Parser | `compiler/parser/parser.c`, `compiler/parser/parser.h` | AST construction |
| AST | `compiler/ast/ast.h`, `compiler/ast/ast.c` | Node definitions |
| Interpreter | `compiler/interpreter/interpreter.c` | Execution |
| CLI | `compiler/cli/cli.c` | Command-line interface |

---

## Notes

- Build: `cd build && make && ./hunnu run examples/main.hn`
- Always test after changes
- Run valgrind on memory-related issues: `valgrind --leak-check=full ./hunnu run test.hn`
- Check for existing tokens before adding new ones (`token.h`)

## Phase 1 Fixes Summary

### Variable Scoping (Scope Stack)
Replaced flat global namespace with a scope chain:
- `Scope` struct with parent pointer for chain traversal
- `scope_lookup()` searches up the chain (variable shadowing works)
- `scope_define()` only defines in current scope (new `let` shadows outer)
- Block bodies use current scope (no new scope for while/for loops)
- Block statements create new scope (for `let` declarations inside `{}`)

### break/continue
- Added `AST_BREAK_STMT` and `AST_CONTINUE_STMT` AST node types
- Parser handles `break` and `continue` keywords
- Interpreter uses `has_break` and `has_continue` flags
- While/for loops check flags after body execution
- `continue` clears flag and re-evaluates condition
- `break` exits the loop entirely

### Memory Fixes
- **Parser dangling pointer**: `ast_call_expr_create()` for `len()` was receiving address of local variable — fixed by allocating args array on heap
- **value_copy for strings**: Deep copies string to prevent double-free
- **value_free for arrays**: Only frees array_elements pointer, not individual elements (shallow ownership)