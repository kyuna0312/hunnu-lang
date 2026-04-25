# Future Improvements — Hunnu Language

Tracked ideas and planned features, ordered roughly by priority.

---

## Critical / Foundation Fixes

### 1. Scoped Environments (Variable Scoping)
**Problem:** The interpreter uses a single flat array for all variables. Functions share the global scope, so local variables inside a function pollute the global namespace and recursive calls break.

**Fix:** Replace the flat `names[]`/`values[]` arrays with a scope stack. Push a new frame on function call, pop it on return.

```c
typedef struct Scope {
    char** names;
    Value* values;
    size_t count;
    size_t capacity;
    struct Scope* parent;
} Scope;
```

Lookup walks up the chain; define writes to the current frame.

### 2. First-Class Function Calls
**Problem:** `AST_FN_DECL` nodes are executed immediately when encountered — the interpreter just runs the body. There is no mechanism to call a function by name (`add(5, 3)` is parsed into `AST_CALL_EXPR` but `interpreter_evaluate` has no case for it).

**Fix:**
- Store function declarations in the environment as a `VALUE_FN` variant holding `fn_decl` node pointer.
- Add `AST_CALL_EXPR` case to `interpreter_evaluate`: look up the function, push a new scope, bind arguments to parameter names, execute body, pop scope, return `return_value`.

### 3. `break` / `continue` Signal Propagation
**Problem:** `break` and `continue` are parsed but the interpreter has no signal mechanism for them. Only `has_return` exists. Loops cannot be exited early with `break`.

**Fix:** Add `has_break` and `has_continue` flags to `Interpreter`. Check and clear them at the loop boundary, same pattern as `has_return`.

---

## Language Features

### 4. Arrays
Tokens `TOKEN_LBRACKET` and `TOKEN_RBRACKET` already exist in the lexer. Need:
- `AST_ARRAY_EXPR` — array literal `[1, 2, 3]`
- `AST_INDEX_EXPR` — index access `arr[i]`
- `VALUE_ARRAY` variant in `Value` (heap-allocated `Value*` with length)
- Interpreter cases for creation and indexing
- Bounds-check with runtime error on out-of-range

### 5. String Operations
- **Concatenation**: `"Hello" + " World"` — add string+string branch to the `TOKEN_PLUS` case in `interpreter_evaluate`
- **Built-in `len()`**: Add to a built-in function table checked before user-defined lookup
- **Indexing**: `s[i]` returns single-character string (shares `AST_INDEX_EXPR` with arrays)

### 6. Floating-Point Numbers
Add `TOKEN_FLOAT_LITERAL`, `VALUE_FLOAT`, and float arithmetic. `lexer_read_number` needs a `.` branch.

### 7. Compound Assignment Operators
`+=`, `-=`, `*=`, `/=` — syntactic sugar, desugar in parser to `AST_ASSIGN` wrapping a `AST_BINARY_EXPR`.

### 8. `else if` Chains
Parser currently supports `else { ... }` only. Allow `else if condition { ... }` by recursively parsing if-statements as the else branch.

### 9. `null` / `nil` Literal
Expose `VALUE_NONE` as a first-class literal so programmers can explicitly express "no value".

---

## Error Handling & Developer Experience

### 10. Proper Error Recovery in Parser
Currently on a parse error the parser likely panics or produces garbage. Add a `parser_synchronize()` that skips to the next statement boundary, allowing multiple errors to be reported in one run.

### 11. Runtime Error Type with Line Info
`fprintf(stderr, "Error: ...")` messages have no line number. Pass `ASTNode->line` and `ASTNode->column` through to error messages:
```
runtime error at line 7:4 — undefined variable 'x'
```

### 12. `--debug` / `--ast` CLI Flag
Add a flag to `cli.c` that calls `ast_print()` on the parsed tree before interpreting. Useful for debugging parser output without needing a separate tool.

---

## Longer-Term / Stretch Goals

### 13. Type System
Optional static types: `let x: int = 5`, `fn add(a: int, b: int) -> int`. Start with annotation-only (no enforcement), add type-checking pass later.

### 14. Standard Library
A small set of built-in functions callable from Hunnu:
- `len(s)` — string/array length
- `str(n)` — int to string
- `int(s)` — string to int
- `input()` — read a line from stdin

### 15. Bytecode Compiler + VM
Replace the tree-walk interpreter with:
1. A bytecode compiler that emits a flat instruction array
2. A stack-based VM that executes it

Benefits: faster execution, easier to add closures, serializable bytecode.

### 16. Multi-line Strings / String Escapes
Support `\n`, `\t`, `\\`, `\"` escape sequences in string literals. Currently the lexer copies raw bytes with no escape handling.

### 17. Modules / Import System
`import "math"` or `import "./utils.hn"` — load and execute another `.hn` file, merging its top-level function definitions into the current scope.

---

## Refactoring Notes

- `interpreter_execute_statement` and `interpreter_evaluate` overlap for `AST_ASSIGN` — both handle it. After scoped environments are added, unify into a single evaluation path.
- `lexer_read_string` is called from a different code path than the main `switch` in `lexer_next_token` — the `'"'` case in the switch calls it directly but doesn't advance first, unlike other branches. Audit for off-by-one.
- `lexer_skip_whitespace` is defined but the main loop in `lexer_next_token` reimplements the same logic inline. Remove the duplicate.
