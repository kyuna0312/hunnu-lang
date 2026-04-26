# Changelog

All notable changes to the Hunnu language project.

---

## [Unreleased]

### Phase 4: Bytecode Compiler + VM ✅
*April 2025*

#### Bytecode Instruction Set
- Added `opcodes.h` with full instruction enumeration
- Instructions for constants, arithmetic, comparison, control flow
- Local variable access, arrays, indexing, function calls

```c
// Example bytecode
OP_CONSTANT_INT 10       // push integer
OP_CONSTANT_INT 20       // push another
OP_ADD                 // add them
OP_SET_LOCAL 0          // store in local 0
OP_GET_LOCAL 0          // retrieve local 0
OP_GET_INDEX           // array indexing
OP_CALL                // function call
OP_RETURN             // return from function
```

#### Bytecode Compiler
- Added `compiler/compiler.c` - AST to bytecode compilation
- Walks AST and emits corresponding bytecode
- Manages constant pool for strings
- Supports: program, function, block, variable declarations, if/while, arrays, print

#### Virtual Machine
- Added `compiler/vm.c` - stack-based VM execution
- Value stack (256 slots max)
- Local variable storage
- Builtin function dispatch (print, input, to_int, to_float, to_str)
- Bytecode interpreter loop

#### CLI Integration
- Added `build` command to output bytecode
- Added `--vm` flag to `run` command for VM execution

```bash
./hunnu build examples/main.hn   # output bytecode
./hunnu run examples/main.hn --vm  # run with VM
```

#### Files
- `compiler/vm/opcodes.h` - instruction enum
- `compiler/vm/compiler.h` - compiler types
- `compiler/vm/compiler.c` - bytecode compiler
- `compiler/vm/vm.h` - VM header
- `compiler/vm/vm.c` - VM execution
- Updated `cli/main.c`, `cli/cli.h` - new commands and flags

---

### Phase 3: Standard Library & Dev Experience ✅
*April 2025*

#### Standard Library Functions
- `input()` - read user input from stdin
- `to_str()` - convert to string
- `to_int()` - convert to integer
- `to_float()` - convert to float

#### CLI Improvements
- Added `--debug` / `-d` flag to show tokens and AST

---

### Phase 2: Core Language Features ✅
*April 2025*

#### Compound Assignment
- Added `+=`, `-=`, `*=`, `/=` operators
- Desugared in parser to `x = x + y` form

```hunnu
let x = 10
x += 5   // x = 15
x -= 3   // x = 12
x *= 2   // x = 24
x /= 4   // x = 6
```

#### else if Chains
- Improved `if` statement to support chained `else if`

```hunnu
if x > 90 {
    print("A")
} else if x > 80 {
    print("B")
} else {
    print("C")
}
```

#### Floating-Point Numbers
- Added `TOKEN_FLOAT_LITERAL` and `VALUE_FLOAT` type
- Full arithmetic support with mixed int/float

```hunnu
let pi = 3.14159
let r = 2.0
print(pi * r * r)  // 12.5664
print(10 + 3.5)    // 13.5
```

#### null/nil Literal
- Added `null` and `nil` keywords

```hunnu
let x = null
let y = nil
```

#### String Escapes
- Full escape sequence support: `\n`, `\t`, `\\`, `\"`

---

### Phase 1: Foundation Fixes ✅
*April 2025*

#### Variable Scoping (Scope Stack)
- Replaced flat global namespace with scope chain
- Block-scoped variables with proper shadowing

#### break/continue
- Full implementation with signal flags

#### Memory Fixes
- Parser dangling pointer bug fixed
- String deep copy implemented

---

### Mongolian Keywords ✅
*Added: April 2025*

Hunnu now supports both English and Mongolian (Cyrillic) keywords:

| English | Mongolian |
|---------|-----------|
| `let` | `хувьсагч` |
| `fn` | `функц` |
| `if` | `хэрвээ` |
| `true` | `үнэн` |
| `false` | `худал` |
| `print` | `хэвлэх` |
| `while` | `давталт` |
| `for` | `тооллого` |
| `return` | `буцаах` |
| `break` | `зогсоох` |
| `continue` | `үргэлжлүүлэх` |
| `null` | `хоосон` |

```hunnu
функц main() {
    хувьсагч тоо = 10
    хэвлэх(тоо)
}
```

---

## Previous Features
- Variables: `let x = 5`
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `>`, `<`, `>=`, `<=`, `==`, `!=`
- Boolean: `and`, `or`, `not`
- If/else statements
- While loops
- For loops
- Functions
- Return statements
- Print
- Variable reassignment
- Arrays + indexing
- String concatenation
- `len()` built-in function