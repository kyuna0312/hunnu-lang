pub mod opcodes;
pub mod value;
pub mod vm;

pub use value::Value;
pub use vm::{Function, Program, VM};

#[cfg(test)]
mod tests {
    use super::*;

    fn make_program(bytecode: Vec<u8>, constants: Vec<Value>) -> Program {
        Program {
            bytecode,
            constants,
        }
    }

    #[test]
    fn test_integer_addition() {
        // OP_CONSTANT_INT 1, OP_CONSTANT_INT 2, OP_ADD, OP_HALT
        let bytecode = vec![
            opcodes::OpCode::ConstantInt as u8,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            opcodes::OpCode::ConstantInt as u8,
            2,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            opcodes::OpCode::Add as u8,
            opcodes::OpCode::Halt as u8,
        ];
        let program = make_program(bytecode, vec![]);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }

    #[test]
    fn test_float_addition() {
        // OP_CONSTANT_FLOAT 1.5, OP_CONSTANT_FLOAT 2.5, OP_ADD, OP_HALT
        let f1_bits = (1.5f64).to_bits();
        let f2_bits = (2.5f64).to_bits();
        let bytecode = vec![
            opcodes::OpCode::ConstantFloat as u8,
            (f1_bits & 0xFF) as u8,
            ((f1_bits >> 8) & 0xFF) as u8,
            ((f1_bits >> 16) & 0xFF) as u8,
            ((f1_bits >> 24) & 0xFF) as u8,
            ((f1_bits >> 32) & 0xFF) as u8,
            ((f1_bits >> 40) & 0xFF) as u8,
            ((f1_bits >> 48) & 0xFF) as u8,
            ((f1_bits >> 56) & 0xFF) as u8,
            opcodes::OpCode::ConstantFloat as u8,
            (f2_bits & 0xFF) as u8,
            ((f2_bits >> 8) & 0xFF) as u8,
            ((f2_bits >> 16) & 0xFF) as u8,
            ((f2_bits >> 24) & 0xFF) as u8,
            ((f2_bits >> 32) & 0xFF) as u8,
            ((f2_bits >> 40) & 0xFF) as u8,
            ((f2_bits >> 48) & 0xFF) as u8,
            ((f2_bits >> 56) & 0xFF) as u8,
            opcodes::OpCode::Add as u8,
            opcodes::OpCode::Halt as u8,
        ];
        let program = make_program(bytecode, vec![]);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }

    #[test]
    fn test_string_concatenation() {
        // OP_CONSTANT_STRING 0, OP_CONSTANT_STRING 1, OP_ADD, OP_HALT
        let bytecode = vec![
            opcodes::OpCode::ConstantString as u8,
            0,
            opcodes::OpCode::ConstantString as u8,
            1,
            opcodes::OpCode::Add as u8,
            opcodes::OpCode::Halt as u8,
        ];
        let constants = vec![
            Value::String("Hello ".to_string()),
            Value::String("World".to_string()),
        ];
        let program = make_program(bytecode, constants);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }

    #[test]
    fn test_comparison() {
        // OP_CONSTANT_INT 5, OP_CONSTANT_INT 3, OP_GREATER, OP_HALT
        let bytecode = vec![
            opcodes::OpCode::ConstantInt as u8,
            5,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            opcodes::OpCode::ConstantInt as u8,
            3,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            opcodes::OpCode::Greater as u8,
            opcodes::OpCode::Halt as u8,
        ];
        let program = make_program(bytecode, vec![]);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }

    #[test]
    fn test_jump_if_false() {
        // if (false) { 1 } else { 2 } -> should push 2
        // Bytecode: FALSE, JUMP_IF_FALSE (offset to 2), CONST_INT 1, JUMP (offset to HALT), CONST_INT 2, HALT
        // Positions: 0:FALSE, 1:JUMP_IF_FALSE, 2-9:offset, 10:CONST_INT, 11-18:1, 19:JUMP, 20-27:offset, 28:CONST_INT, 29-36:2, 37:HALT
        // JUMP_IF_FALSE should jump to position 28 (CONST_INT 2), offset = 28 - 10 = 18
        // JUMP should jump to position 37 (HALT), offset = 37 - 28 = 9
        let bytecode = vec![
            opcodes::OpCode::False as u8,       // [0]
            opcodes::OpCode::JumpIfFalse as u8, // [1]
            18,
            0,
            0,
            0,
            0,
            0,
            0,
            0,                                  // [2-9] jump to [28]
            opcodes::OpCode::ConstantInt as u8, // [10]
            1,
            0,
            0,
            0,
            0,
            0,
            0,
            0,                           // [11-18]
            opcodes::OpCode::Jump as u8, // [19]
            9,
            0,
            0,
            0,
            0,
            0,
            0,
            0,                                  // [20-27] jump to [37]
            opcodes::OpCode::ConstantInt as u8, // [28]
            2,
            0,
            0,
            0,
            0,
            0,
            0,
            0,                           // [29-36]
            opcodes::OpCode::Halt as u8, // [37]
        ];
        let program = make_program(bytecode, vec![]);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }
}
