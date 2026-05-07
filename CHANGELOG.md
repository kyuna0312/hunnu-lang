# Changelog

All notable changes to the Hunnu language project.

---

## [Unreleased]

### Month 2: FFI Ecosystem + Standard Library ✅
*May-June 2026*

#### FFI Improvements
- ✅ FFI string returns (`extern fn` returning `str`)
- ✅ FFI float arguments (float params in extern calls)
- ✅ Rust FFI boundary (call Rust functions from Hunnu)
- Added `extern fn` examples: `test_str_return.hn`, `test_float_ffi.hn`, `test_rust_ffi.hn`

#### Error Handling
- Added `try`/`catch`/`finally` syntax
- New token types: `TOKEN_TRY`, `TOKEN_CATCH`, `TOKEN_FINALLY`
- New AST node: `AST_TRY_STMT`
- Parser supports `try { } catch { }` blocks
- Interpreter executes try/catch/finally blocks

#### Module System
- Fixed import path resolution for both:
  - File imports: `import "stdlib/math.hn"`
  - Module imports: `import std.math` (converts to `stdlib/math.hn`)
- Improved `resolve_import_path()` function

#### Standard Library Expansion
- `stdlib/libc.hn` - C standard library bindings (puts, printf, strlen, etc.)
- `stdlib/math.hn` - Math functions (pow, sqrt, sin, cos, tan, fabs)
- `stdlib/io.hn` - I/O functions (println, read_line)
- `stdlib/array.hn` - Array utilities (map, filter, reduce, length)
- `stdlib/string.hn` - String utilities (to_upper, to_lower, contains, trim, split, join)
- `stdlib/fs.hn` - Filesystem functions (exists, read_file, write_file)
- `stdlib/time.hn` - Time functions (now, timestamp)

#### Python Bindings (PyO3)
- Created `bindings/python/` directory structure
- Added `Cargo.toml` with PyO3 dependency
- Added `src/lib.rs` with Python module skeleton

---

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

## [0.3.0] - 2026-07-XX

### Month 3: AOT Compiler Foundation ✅
*July 2026*

#### Structs/Records
- Added `type` keyword for struct definitions: `type Point = { x: int, y: int }`
- New token types: `TOKEN_TYPE`, `TOKEN_DOT`, `TOKEN_AMPERSAND`
- New AST nodes: `AST_TYPE_DECL`, `AST_FIELD_ACCESS`, `AST_ADDRESS_OF`, `AST_DEREFERENCE`
- Parser support for struct declarations and field access (`point.x`)
- Pointer support: `&x` (address-of) and `*p` (dereference)

#### Rust Compiler Frontend
- Created `compiler-rust/` directory with Cargo project
- Ported lexer to Rust (`compiler-rust/src/lexer.rs`)
- Ported parser to Rust (`compiler-rust/src/parser.rs`)
- Rust AST definitions (`compiler-rust/src/ast.rs`)
- LLVM IR codegen skeleton (`compiler-rust/src/codegen.rs`)

#### Value Type Extensions
- Extended `Value` type with `VALUE_STRUCT` and `VALUE_POINTER`
- Added struct fields support: `struct_fields`, `struct_field_count`, `struct_type`
- Interpreter handles new AST nodes (basic implementation)

#### CLI Changes
- Added `compile` command: `./hunnu compile file.hn -o output`
- Updated `--help` to show new Month 3 features
- Compile command uses system C compiler as temporary fallback

#### Build System
- Build succeeds with new token types and AST nodes
- Warning fixes for unhandled enum values (suppressed with TODO)

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