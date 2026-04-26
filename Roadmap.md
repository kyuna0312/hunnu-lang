# Hunnu Roadmap

The future of Hunnu language — features, fixes, and goals.

---

## Current Status ✅

Working features:
- Variables (`let`)
- Functions (`fn`)
- If/Else
- While loops (`while`)
- For loops (`for`)
- Break/Continue
- Arithmetic (`+`, `-`, `*`, `/`, `%`)
- Comparison (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- Print statements
- Arrays (syntax: `[1, 2, 3]` + indexing)
- String concatenation (`+`)

Known bugs:
- Array indexing (index out of bounds on valid indices)
- String memory issues

---

## Phase 1: Bug Fixes 🔴

### 1.1 Array Index Bug
**Priority:** Critical
**Issue:** Index out of bounds error on valid indices (0-4 for 5 elements)
**File:** `compiler/interpreter/interpreter.c` - AST_INDEX_EXPR case

### 1.2 String Memory 
**Priority:** Critical
**Issue:** Double-free when using strings with variables
**Fix:** Proper value ownership/reference counting

---

## Phase 2: Core Features 🟡

### 2.1 Scoped Environments
**Priority:** High
**Problem:** All variables share global scope
**Solution:** Add scope stack for function-local variables

### 2.2 First-Class Function Calls
**Priority:** High  
**Problem:** Functions execute immediately, can't pass as values
**Solution:** Store functions as VALUES, add proper call semantics

### 2.3 Break/Continue Implementation
**Priority:** High
**Problem:** Parsed but no signal mechanism in interpreter
**Solution:** Add has_break/has_continue flags

---

## Phase 3: Data Types 🟢

### 3.1 Floating-Point Numbers
- `TOKEN_FLOAT_LITERAL`
- `VALUE_FLOAT`
- Float arithmetic

### 3.2 Null/Nil
- Expose `VALUE_NONE` as first-class literal

### 3.3 Multi-dimensional Arrays
- Array of arrays: `[[1,2], [3,4]]`

---

## Phase 4: Strings 🟢

### 4.1 String Escapes
- Support `\n`, `\t`, `\\`, `\"`

### 4.2 String Slicing
- `s[0:5]` substring extraction

### 4.3 More String Functions
- `str(n)` - int to string
- `int(s)` - string to int
- `input()` - read user input

---

## Phase 5: Language Features 🟡

### 5.1 Compound Assignment
- `+=`, `-=`, `*=`, `/=`

### 5.2 Else If Chains
- `if ... else if ... else`

### 5.3 Type Annotations (Optional)
- `let x: int = 5`
- `fn add(a: int, b: int) -> int`

---

## Phase 6: Developer Experience 🟢

### 6.1 Better Error Messages
- Include line/column numbers in runtime errors

### 6.2 Parser Recovery
- Report multiple errors in one run

### 6.3 Debug CLI Flag
- `--ast` flag to print parsed tree

---

## Phase 7: Long-term 🔵

### 7.1 Bytecode Compiler + VM
- Replace tree-walk with bytecode compilation
- Faster execution
- Serializable bytecode

### 7.2 Modules/Import System
- `import "module"`

### 7.3 Standard Library
- Built-in math, string, array functions

---

## Priority Order

| Priority | Item |
|----------|------|
| 🔴 Critical | Array indexing fix |
| 🔴 Critical | String memory fix |
| 🟡 High | Scoped environments |
| 🟡 High | First-class functions |
| 🟡 High | Break/continue signals |
| 🟢 Medium | Float support |
| 🟢 Medium | String escapes |
| 🟢 Medium | Null literal |
| 🟡 Low | Type annotations |
| 🔵 Stretch | Bytecode VM |
| 🔵 Stretch | Modules |

---

## Contributing

Ideas welcome! Open an issue or pull request.

---

## See Also

- [`future-improvements.md`](future-improvements.md) - Detailed technical notes
- [`task.md`](task.md) - Session-by-session tasks
- [`plan.md`](plan.md) - Stream schedule