# Today's Task: Arrays & Strings Implementation

## Date
Today's session

## Goal
Add arrays and string operations to Hunnu language

---

## Completed Items

### 1. Token Definitions ✅
- `TOKEN_LBRACKET`, `TOKEN_RBRACKET` - already existed in `compiler/lexer/token.h`

### 2. AST Nodes ✅
- Added `AST_ARRAY_EXPR` to `compiler/ast/ast.h`
- Added `AST_INDEX_EXPR` to `compiler/ast/ast.h`
- Added `AST_STRING_CONCAT` to `compiler/ast/ast.h`
- Added union members for array_expr, index_expr, string_concat
- Added creation functions in `compiler/ast/ast.c`
- Added name strings in type_names array

### 3. Parser ✅
- Added array literal parsing: `[expr1, expr2, ...]`
- Added array indexing: `array[index]`
- Added `len()` function call parsing

### 4. Interpreter ✅
- Added VALUE_ARRAY type to Value enum in `interpreter.h`
- Added array_elements field for array storage
- Added array expression evaluation
- Added index expression evaluation
- Added string concatenation (`str1 + str2`)
- Added `len()` function for strings and arrays

---

## Known Issues

### Array Index Bug
**Problem**: Index out of bounds error on valid indices (e.g., index 0-4 for 5 elements)

**Status**: Needs debugging - likely an off-by-one or integer conversion issue

### String Memory Bugs
**Problem**: Double-free or corruption when using strings with variables

**Root Cause**: Complex value copying and ownership in interpreter

**Status**: Needs proper memory management (reference counting or ownership model)

---

## Test Results

| Test | Expected | Got |
|------|----------|-----|
| `let arr = [10,20,30]; print(arr[0])` | 10 | Builds but index bug |
| `let s = "hello"; print(len(s))` | 5 | Memory corruption |
| `"a" + "b"` | "ab" | Works ✅ |

---

## Current Language Features

- Variables with `let`
- Print statements
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `>`, `<`, `>=`, `<=`
- If/else statements
- While loops: `while(condition) { body }`
- For loops: `for(init; condition; update) { body }`
- Return statements: `return expression`
- Variable reassignment: `x = new_value`
- **Arrays**: `[1, 2, 3]` (syntax works, indexing needs fix)
- **String concat**: `"a" + "b"` (works with literals)
- **len()**: function defined (needs memory fix)

---

## Syntax Reference

```hunnu
// Array creation
let numbers = [10, 20, 30, 40, 50]

// Array indexing
print(numbers[0])  // First element

// String concatenation
let greeting = "Hello, "
let name = "World"
let message = greeting + name

// String length
let s = "Hunnu"
let length = len(s)
```

---

## Next
See `plan.md` for tomorrow's session: Break/Continue, I/O basics, Multi-dimensional arrays