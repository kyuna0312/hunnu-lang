# Hunnu 2-Hour Stream Plan: Break/Continue & More Features

## Stream Overview

- **Topic:** Continue fixing arrays, add break/continue, I/O basics
- **Duration:** 2 hours
- **Audience:** Beginner-friendly

---

## Pre-Stream Checklist

- [ ] Hunnu project open in editor
- [ ] Example file `main.hn` ready to demo
- [ ] Build verified (`cd build && make`)
- [ ] Whiteboard/diagram ready for concepts
- [ ] Terminal open, running `./build/hunnu run examples/main.hn`

---

## Current Language State (Completed)

The following features are now working:
- Variables with `let`
- Print statements
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `>`, `<`, `>=`, `<=`
- If/else statements
- **While loops**: `while(condition) { body }`
- **For loops**: `for(init; condition; update) { body }`
- **Return statements**: `return expression`
- Variable reassignment: `x = new_value`
- **Arrays** (syntax): `[1, 2, 3]` with indexing `arr[i]`
- **String concat**: `"Hello" + "World"`
- **len()** function: for strings and arrays

---

## Bug Fixes (Priority)

### 1. Array Index Bug 🔴
**Problem**: Index out of bounds error on valid indices (0-4 for 5 elements)
**Location**: `interpreter.c` - AST_INDEX_EXPR case

### 2. String Memory Bugs 🔴
**Problem**: Double-free or corruption when using strings with variables
**Root Cause**: Complex value copying and ownership in interpreter

---

## Hour 1: Bug Fixes (55 min)

| Time | Segment | Content |
|------|---------|---------|
| 0:00-0:10 | Debug | Fix array indexing bug |
| 0:10-0:25 | Debug | Fix string memory issues |
| 0:25-0:40 | Test | Verify arrays and strings work |
| 0:40-0:55 | Demo | Live demo with arrays + strings |

**Break: 5 min**

---

## Hour 2: New Features (55 min)

| Time | Segment | Content |
|------|---------|---------|
| 1:00-1:15 | Code | **Break/Continue** - loop control |
| 1:15-1:30 | Code | **Input basics** - read user input |
| 1:30-1:40 | Concept | Multi-dimensional arrays |
| 1:40-1:50 | Code | String slicing basics |
| 1:50-1:55 | Demo | Combined demo |
| 1:55-2:00 | Wrap | Summary, Q&A |

---

## Files to Modify

| File | Changes |
|------|---------|
| `compiler/interpreter/interpreter.c` | Fix array indexing, string memory |
| `compiler/lexer/token.h` | Add `TOKEN_BREAK`, `TOKEN_CONTINUE` |
| `compiler/parser/parser.c` | Parse break/continue |
| `compiler/interpreter/interpreter.c` | Execute break/continue |
| `examples/main.hn` | Demo with new features |

---

## Syntax Examples

### Break/Continue
```hunnu
let i = 0
while i < 10 {
    i = i + 1
    if i == 5 {
        break  // exits loop
    }
    if i == 3 {
        continue  // skip to next iteration
    }
    print(i)
}
```

### Final Demo Program (After Fixes)
```hunnu
fn print_array(arr, size) {
    let i = 0
    while i < size {
        print(arr[i])
        i = i + 1
    }
}

fn main() {
    let numbers = [10, 20, 30, 40, 50]
    print("Array elements:")
    print_array(numbers, 5)
    
    let first = numbers[0]
    let last = numbers[4]
    print("First + Last:")
    print(first + last)
    
    let name = "Hunnu"
    let message = "Language: " + name
    print(message)
    print(len(message))
}
```

---

## Next Stream Preview

- Function parameters by reference
- Structs/records
- Type system basics
- Error handling
- Modules/imports

---

## Notes

- Explain each step before coding
- Show errors and how to fix them
- Take it slow for beginners
- Q&A breaks after each segment