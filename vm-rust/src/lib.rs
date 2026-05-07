pub mod opcodes;
pub mod value;
pub mod vm;

pub use value::Value;
pub use vm::{Function, Program, VM};

/// Helper to create a ConstantInt bytecode sequence
pub fn const_int(value: i64) -> Vec<u8> {
    let mut bytes = vec![opcodes::OpCode::ConstantInt as u8];
    for i in 0..8 {
        bytes.push((value >> (i * 8)) as u8);
    }
    bytes
}

/// Helper to create a ConstantFloat bytecode sequence
pub fn const_float(value: f64) -> Vec<u8> {
    let mut bytes = vec![opcodes::OpCode::ConstantFloat as u8];
    let bits = value.to_bits();
    for i in 0..8 {
        bytes.push(((bits >> (i * 8)) & 0xFF) as u8);
    }
    bytes
}

/// Helper to create a ConstantString bytecode sequence
pub fn const_string(idx: u8) -> Vec<u8> {
    vec![opcodes::OpCode::ConstantString as u8, idx]
}

#[cfg(test)]
mod tests {
    use super::*;

    fn run_vm(bytecode: Vec<u8>, constants: Vec<Value>) -> (Result<(), String>, String) {
        let program = Program {
            bytecode,
            constants,
        };
        let mut vm: VM<Vec<u8>> = VM::new();
        let result = vm.run(&program);
        let output = vm.take_output();
        (result, output)
    }

    #[test]
    fn test_integer_addition() {
        let mut bytecode = Vec::new();
        bytecode.extend(const_int(1));
        bytecode.extend(const_int(2));
        bytecode.push(opcodes::OpCode::Add as u8);
        bytecode.push(opcodes::OpCode::Halt as u8);
        let (result, _) = run_vm(bytecode, vec![]);
        assert!(result.is_ok());
    }

    #[test]
    fn test_float_addition() {
        let mut bytecode = Vec::new();
        bytecode.extend(const_float(1.5));
        bytecode.extend(const_float(2.5));
        bytecode.push(opcodes::OpCode::Add as u8);
        bytecode.push(opcodes::OpCode::Halt as u8);
        let (result, _) = run_vm(bytecode, vec![]);
        assert!(result.is_ok());
    }

    #[test]
    fn test_string_concatenation() {
        let mut bytecode = Vec::new();
        bytecode.extend(const_string(0));
        bytecode.extend(const_string(1));
        bytecode.push(opcodes::OpCode::Add as u8);
        bytecode.push(opcodes::OpCode::Halt as u8);
        let constants = vec![
            Value::String("Hello ".to_string()),
            Value::String("World".to_string()),
        ];
        let (result, _) = run_vm(bytecode, constants);
        assert!(result.is_ok());
    }

    #[test]
    fn test_comparison() {
        let mut bytecode = Vec::new();
        bytecode.extend(const_int(5));
        bytecode.extend(const_int(3));
        bytecode.push(opcodes::OpCode::Greater as u8);
        bytecode.push(opcodes::OpCode::Halt as u8);
        let (result, _) = run_vm(bytecode, vec![]);
        assert!(result.is_ok());
    }

    #[test]
    fn test_jump_if_false() {
        let mut bytecode = Vec::new();
        bytecode.push(opcodes::OpCode::False as u8);
        bytecode.push(opcodes::OpCode::JumpIfFalse as u8);
        for i in 0..8 {
            bytecode.push(if i == 0 { 18 } else { 0 });
        }
        bytecode.extend(const_int(1));  // This should be skipped
        bytecode.push(opcodes::OpCode::Jump as u8);
        for i in 0..8 {
            bytecode.push(if i == 0 { 9 } else { 0 });
        }
        bytecode.extend(const_int(2));  // This should be executed
        bytecode.push(opcodes::OpCode::Halt as u8);
        let (result, _) = run_vm(bytecode, vec![]);
        assert!(result.is_ok());
    }

    #[test]
    fn test_print_output() {
        let mut bytecode = Vec::new();
        bytecode.extend(const_int(42));
        bytecode.extend(const_string(0));  // "print"
        bytecode.push(opcodes::OpCode::Call as u8);
        bytecode.push(1);  // 1 arg
        bytecode.push(opcodes::OpCode::Halt as u8);
        let constants = vec![Value::String("print".to_string())];
        let (result, output) = run_vm(bytecode, constants);
        assert!(result.is_ok(), "VM run failed: {:?}", result);
        assert!(output.contains("42"), "Expected output to contain '42', got: '{}'", output);
    }

    #[test]
    fn test_arithmetic_and_print() {
        let mut bytecode = Vec::new();
        bytecode.extend(const_int(2));
        bytecode.extend(const_int(3));
        bytecode.push(opcodes::OpCode::Add as u8);
        bytecode.extend(const_string(0));  // "print"
        bytecode.push(opcodes::OpCode::Call as u8);
        bytecode.push(1);  // 1 arg
        bytecode.push(opcodes::OpCode::Halt as u8);
        let constants = vec![Value::String("print".to_string())];
        let (result, output) = run_vm(bytecode, constants);
        assert!(result.is_ok(), "VM run failed: {:?}", result);
        assert!(output.contains("5"), "Expected output to contain '5', got: '{}'", output);
    }

    #[test]
    fn test_user_function_call() {
        let mut bytecode = Vec::new();

        // [0-10] Define function "foo" at entry_point=20
        bytecode.push(opcodes::OpCode::DefineFn as u8);
        bytecode.push(0);  // name_idx=0 ("foo")
        // entry_point=20 (function body starts after main program)
        for i in 0..8 {
            bytecode.push(if i == 0 { 20 } else { 0 });
        }
        bytecode.push(0);  // arg_count=0

        // [11-12] Main: push "foo"
        bytecode.extend(const_string(0));  // "foo" (index 0)
        // [13-14] Call foo with 0 args
        bytecode.push(opcodes::OpCode::Call as u8);
        bytecode.push(0);
        // [15-16] Push "print"
        bytecode.extend(const_string(1));  // "print" (index 1)
        // [17-18] Call print with 1 arg
        bytecode.push(opcodes::OpCode::Call as u8);
        bytecode.push(1);
        // [19] HALT
        bytecode.push(opcodes::OpCode::Halt as u8);

        // [20-28] Function body: CONST_INT 10
        bytecode.extend(const_int(10));
        // [29] RETURN
        bytecode.push(opcodes::OpCode::Return as u8);

        let constants = vec![
            Value::String("foo".to_string()),
            Value::String("print".to_string()),
        ];
        let (result, output) = run_vm(bytecode, constants);
        assert!(result.is_ok(), "VM run failed: {:?}", result);
        assert!(output.contains("10"), "Expected output to contain '10', got: '{}'", output);
    }
}

// C FFI interface for Rust VM
#[no_mangle]
pub extern "C" fn hunnu_vm_run(bytecode_ptr: *const u8, bytecode_len: usize,
                                constants_ptr: *const u8, constants_len: usize) -> i32 {
    use std::io::Write;
    use std::slice;

    if bytecode_ptr.is_null() || constants_ptr.is_null() {
        return -1;
    }

    let bytecode = unsafe {
        slice::from_raw_parts(bytecode_ptr, bytecode_len)
    };

    let constants_data = unsafe {
        slice::from_raw_parts(constants_ptr, constants_len)
    };

    // Parse constants from the binary format
    let mut constants = Vec::new();
    let mut pos = 0;

    // First 8 bytes: constants count
    if constants_data.len() < 8 {
        return -1;
    }
    let count = i64_from_bytes(&constants_data[0..8]) as usize;
    pos = 8;

    for _ in 0..count {
        if pos >= constants_data.len() {
            return -1;
        }
        let value_type = constants_data[pos];
        pos += 1;

        match value_type {
            0 => {
                if pos + 8 > constants_data.len() {
                    return -1;
                }
                let val = i64_from_bytes(&constants_data[pos..pos + 8]);
                pos += 8;
                constants.push(Value::Int(val));
            }
            1 => {
                if pos + 8 > constants_data.len() {
                    return -1;
                }
                let bits = u64_from_bytes(&constants_data[pos..pos + 8]);
                pos += 8;
                constants.push(Value::Float(f64::from_bits(bits)));
            }
            2 => {
                if pos + 8 > constants_data.len() {
                    return -1;
                }
                let str_len = i64_from_bytes(&constants_data[pos..pos + 8]) as usize;
                pos += 8;
                if pos + str_len > constants_data.len() {
                    return -1;
                }
                let s = String::from_utf8_lossy(&constants_data[pos..pos + str_len]).to_string();
                pos += str_len;
                constants.push(Value::String(s));
            }
            3 => {
                if pos >= constants_data.len() {
                    return -1;
                }
                let b = constants_data[pos] != 0;
                pos += 1;
                constants.push(Value::Bool(b));
            }
            4 => constants.push(Value::None),
            _ => constants.push(Value::None),
        }
    }

    let program = Program {
        bytecode: bytecode.to_vec(),
        constants,
    };

    let mut vm: VM<Vec<u8>> = VM::new();
    match vm.run(&program) {
        Ok(_) => {
            // Print captured output to stdout
            let output = vm.take_output();
            if !output.is_empty() {
                print!("{}", output);
                std::io::stdout().flush().unwrap_or(());
            }
            0
        },
        Err(_) => -1,
    }
}

fn i64_from_bytes(bytes: &[u8]) -> i64 {
    let mut val: i64 = 0;
    for i in 0..8 {
        val |= (bytes[i] as i64) << (i * 8);
    }
    val
}

fn u64_from_bytes(bytes: &[u8]) -> u64 {
    let mut val: u64 = 0;
    for i in 0..8 {
        val |= (bytes[i] as u64) << (i * 8);
    }
    val
}

// Rust FFI functions that can be called from Hunnu via extern fn
// These functions are exported with #[no_mangle] for dynamic loading

/// Example Rust function: returns the square of a number
#[no_mangle]
pub extern "C" fn hunnu_math_square(x: i64) -> i64 {
    x * x
}

/// Example Rust function: returns the sum of two numbers
#[no_mangle]
pub extern "C" fn hunnu_math_add(a: i64, b: i64) -> i64 {
    a + b
}

/// Example Rust function: returns a greeting string (static)
#[no_mangle]
pub extern "C" fn hunnu_get_greeting() -> *const u8 {
    b"Hello from Rust FFI!\0".as_ptr()
}

/// Example Rust function: computes factorial
#[no_mangle]
pub extern "C" fn hunnu_math_factorial(n: i64) -> i64 {
    if n <= 1 { 1 } else { n * hunnu_math_factorial(n - 1) }
}
