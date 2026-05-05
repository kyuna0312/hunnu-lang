use hunnu_vm::value::Value;
use hunnu_vm::vm::{Program, VM};
use std::io::{Read, stdout, Write};

fn main() {
    let args: Vec<String> = std::env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: {} <bytecode_file>", args[0]);
        std::process::exit(1);
    }

    let filename = &args[1];

    // Read the bytecode file
    let mut file = match std::fs::File::open(filename) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Error opening file '{}': {}", filename, e);
            std::process::exit(1);
        }
    };

    let mut data = Vec::new();
    if let Err(e) = file.read_to_end(&mut data) {
        eprintln!("Error reading file: {}", e);
        std::process::exit(1);
    }

    // Parse the bytecode format:
    // - 8 bytes: bytecode length (i64)
    // - bytecode bytes
    // - 8 bytes: constants count (i64)
    // - for each constant:
    //   - 1 byte: value type (0=int, 1=float, 2=string, 3=bool, 4=none, 5=array)
    //   - depending on type, the value

    if data.len() < 16 {
        eprintln!("Error: Invalid bytecode file (too short)");
        std::process::exit(1);
    }

    let mut pos = 0;

    // Read bytecode length
    let bytecode_len = i64_from_bytes(&data[pos..pos + 8]) as usize;
    pos += 8;

    if pos + bytecode_len > data.len() {
        eprintln!("Error: Invalid bytecode file (bytecode length mismatch)");
        std::process::exit(1);
    }

    let bytecode = data[pos..pos + bytecode_len].to_vec();
    pos += bytecode_len;

    // Debug: print bytecode
    eprintln!(
        "Bytecode ({} bytes): {:?}",
        bytecode.len(),
        &bytecode[..bytecode.len().min(50)]
    );

    // Read constants count
    if pos + 8 > data.len() {
        eprintln!("Error: Invalid bytecode file (missing constants count)");
        std::process::exit(1);
    }

    let constants_count = i64_from_bytes(&data[pos..pos + 8]) as usize;
    pos += 8;

    let mut constants = Vec::new();
    for _ in 0..constants_count {
        if pos >= data.len() {
            eprintln!("Error: Invalid bytecode file (unexpected end of constants)");
            std::process::exit(1);
        }

        let value_type = data[pos];
        pos += 1;

        let value = match value_type {
            0 => {
                // Int
                if pos + 8 > data.len() {
                    eprintln!("Error: Invalid bytecode file (unexpected end of int value)");
                    std::process::exit(1);
                }
                let val = i64_from_bytes(&data[pos..pos + 8]);
                pos += 8;
                Value::Int(val)
            }
            1 => {
                // Float
                if pos + 8 > data.len() {
                    eprintln!("Error: Invalid bytecode file (unexpected end of float value)");
                    std::process::exit(1);
                }
                let bits = u64_from_bytes(&data[pos..pos + 8]);
                pos += 8;
                Value::Float(f64::from_bits(bits))
            }
            2 => {
                // String
                if pos + 8 > data.len() {
                    eprintln!("Error: Invalid bytecode file (unexpected end of string length)");
                    std::process::exit(1);
                }
                let str_len = i64_from_bytes(&data[pos..pos + 8]) as usize;
                pos += 8;

                if pos + str_len > data.len() {
                    eprintln!("Error: Invalid bytecode file (unexpected end of string value)");
                    std::process::exit(1);
                }
                let s = String::from_utf8_lossy(&data[pos..pos + str_len]).to_string();
                pos += str_len;
                Value::String(s)
            }
            3 => {
                // Bool
                if pos >= data.len() {
                    eprintln!("Error: Invalid bytecode file (unexpected end of bool value)");
                    std::process::exit(1);
                }
                let b = data[pos] != 0;
                pos += 1;
                Value::Bool(b)
            }
            4 => Value::None,
            5 => {
                // Array - simplified for now
                if pos + 8 > data.len() {
                    eprintln!("Error: Invalid bytecode file (unexpected end of array length)");
                    std::process::exit(1);
                }
                let arr_len = i64_from_bytes(&data[pos..pos + 8]) as usize;
                pos += 8;
                // Skip array elements for now
                // TODO: properly deserialize array elements
                Value::Array(vec![])
            }
            _ => {
                eprintln!("Error: Unknown value type {}", value_type);
                std::process::exit(1);
            }
        };
        constants.push(value);
    }

    let program = Program {
        bytecode,
        constants,
    };
    let mut vm: VM<std::io::Stdout> = VM::new_with_output(stdout());

    if let Err(e) = vm.run(&program) {
        eprintln!("VM Error: {}", e);
        std::process::exit(1);
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
