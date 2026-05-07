# Bugs Fixed in Hunnu Language

This document tracks bugs discovered and fixed during development.

## Assignment Not Updating Variable Value

**Date**: Session where while/for loops were being implemented.

**Symptom**:
```hunnu
let x = 5
x = 3
print(x)  // Expected: 3, Got: 5
```

**Root Cause**:
The `AST_ASSIGN` case only existed in `interpreter_execute_statement()` which is a void function and doesn't return values. The assignment was being executed but the value wasn't being returned to calling code.

Additionally, there was a duplicate `AST_ASSIGN` case in the wrong function that had `return value;` which caused a compiler error since it's in a void function.

**Fix**:
1. Removed the buggy `AST_ASSIGN` case from `interpreter_execute_statement()`
2. Added proper `AST_ASSIGN` case to `interpreter_evaluate()` that:
   - Evaluates the value expression
   - Updates the variable in the environment via `interpreter_define()`
   - Returns the assigned value

**File Changed**: `compiler/interpreter/interpreter.c`

---

## Parser Required Parentheses for While Loops

**Symptom**:
```hunnu
while x > 0 { ... }  // Error: Expected '(' after 'while'
```

**Fix**: Updated test syntax to use parentheses: `while(x > 0) { ... }`

---

## Previous Issues (from earlier sessions)

- **TOKEN_NEWLINE treated as statement-terminator**: Parser was treating newlines as TOKEN_NEWLINE which broke statement parsing. Fixed by adding `parser_skip_newlines()` to handle newlines between statements.

- **Variable reassignment not working**: Had to modify `interpreter_define()` to check for existing variables and update them instead of creating duplicates.

- **Infinite while loops**: While loop conditions weren't being re-evaluated because assignment inside the loop body wasn't updating the variable. Fixed by ensuring assignment returns values properly.

- **Missing `has_value` in Value struct**: Value struct needed `has_value` field to handle uninitialized values properly in `value_as_bool()`.

- **Missing semicolon handling**: Parser required explicit semicolon consumption after var declarations, expressions, and print statements.

- **Missing assignment parsing**: Had to add `parser_parse_assignment()` function for assignment expressions.

---

## Month 1-2 Bugs (Fixed)

- **Array indexing out of bounds**: Fixed in Phase 1 with proper bounds checking.

- **String memory safety**: Fixed in Phase 1 with proper string handling and dangling pointer fixes.

- **FFI string returns**: Fixed in Month 2 - `extern fn` returning `str` now works correctly.

- **FFI float arguments**: Fixed in Month 2 - `extern fn` with float params now handled properly.

- **Module import path resolution**: Fixed in Month 2 - `import std.math` now correctly resolves to `stdlib/math.hn`.

- **Try/catch parsing**: Implemented in Month 2 - `try { } catch { }` syntax now works.

---

## Current Status

✅ All known bugs fixed
✅ Month 1 (Rust VM Stabilization) - Complete
✅ Month 2 (FFI Ecosystem + Standard Library) - Complete
🚧 Month 3 (AOT Compiler Foundation) - In progress