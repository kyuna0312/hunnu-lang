# Hunnu 2-Hour Stream Plan: Loops & Functions with Return

## Stream Overview

- **Topic:** Adding `while`, `for` loops, and `return` statements to Hunnu
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

## Hour 1: While & For Loops (55 min)

| Time | Segment | Content |
|------|---------|---------|
| 0:00-0:05 | Intro | Welcome, show current Hunnu demo |
| 0:05-0:15 | Concept | Draw flowcharts: while vs for loops |
| 0:15-0:25 | Code | **Add `TOKEN_WHILE`** in `token.h` |
| 0:25-0:35 | Code | **Add `TOKEN_FOR`** in `token.h` + lexer |
| 0:35-0:45 | Code | **Add AST nodes** - `AST_WHILE_STMT`, `AST_FOR_STMT` |
| 0:45-0:55 | Demo | Live demo `while` loop |

**Break: 5 min**

---

## Hour 2: For Loops & Return (55 min)

| Time | Segment | Content |
|------|---------|---------|
| 1:00-1:15 | Code | **Parser for `for` loops** - init/condition/update |
| 1:15-1:30 | Code | **Interpreter for both loops** |
| 1:30-1:45 | Concept | "Why return matters" - whiteboard explanation |
| 1:45-1:55 | Code | **Add `return` statement** - parser + interpreter |
| 1:55-2:00 | Demo | Full demo with both loops + return |

---

## Files to Modify

| File | Changes |
|------|---------|
| `compiler/lexer/token.h` | Add `TOKEN_WHILE`, `TOKEN_FOR`, `TOKEN_RETURN` |
| `compiler/lexer/lexer.c` | Handle `"while"`, `"for"`, `"return"` keywords |
| `compiler/parser/parser.c` | Parse while, for, return statements |
| `compiler/ast/ast.h` | Add `AST_WHILE_STMT`, `AST_FOR_STMT`, `AST_RETURN` |
| `compiler/interpreter/interpreter.c` | Execute loops & return values |
| `examples/main.hn` | Demo with all features |

---

## Syntax Examples

### While Loop
```hunnu
let x = 5
while x > 0 {
    print(x)
    x = x - 1
}
```

### For Loop
```hunnu
for let i = 0; i < 5; i = i + 1 {
    print(i)
}
```

### Functions with Return
```hunnu
fn countdown(n) {
    while n > 0 {
        print(n)
        n = n - 1
    }
    return n
}

fn main() {
    let result = countdown(5)
    print(result)
}
```

### Final Demo Program
```hunnu
fn countdown(n) {
    while n > 0 {
        print(n)
        n = n - 1
    }
    return n
}

fn sum_range(start, end) {
    let total = 0
    for let i = start; i <= end; i = i + 1 {
        total = total + i
    }
    return total
}

fn main() {
    print("Countdown:")
    let remaining = countdown(5)
    
    print("Sum of 1 to 10:")
    let s = sum_range(1, 10)
    print(s)
}
```

---

## Teaching Points

### Why Loops?
- Computers are good at repetition
- `while` = "keep going while condition is true"
- `for` = "start here, count up/down to there"

### Why Return?
- Functions can give back a result
- The `return` keyword sends a value back
- Without return, functions just do actions

### Beginner-Friendly Diagrams

```
WHILE LOOP:
┌──────┐
│condition│
└──┬───┘
   │
   ▼
┌──────┐
│ body  │
└──┬───┘
   │
   └──► back to condition
```

```
FOR LOOP:
┌──────────┐
│ init (i=0)│
└────┬─────┘
     │
┌────▼─────┐
│condition │────No──► exit
│ i < 5    │
└────┬─────┘
     │ Yes
     ▼
┌──────┐
│ body  │
└────┬───┘
     │
┌────▼─────┐
│ update   │
│ i = i + 1│
└────┬─────┘
     │
     └──► back to condition
```

---

## Next Stream Preview

- Arrays and indexing: `arr[0]`, `arr[1]`
- String operations: `len(s)`, `s + t`
- Break and continue statements
- Input/output basics

---

## Notes

- Explain each step before coding
- Show errors and how to fix them
- Take it slow for beginners
- Q&A breaks after each segment