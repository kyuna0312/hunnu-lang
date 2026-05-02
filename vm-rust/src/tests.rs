#[cfg(test)]
mod tests {
    use crate::value::Value;
    use crate::vm::VM;

    fn create_program(bytecode: Vec<u8>, constants: Vec<Value>) -> crate::vm::Program {
        crate::vm::Program {
            bytecode,
            constants,
        }
    }

    #[test]
    fn test_integer_addition() {
        let bytecode = vec![
            1, // OP_CONSTANT_INT
            1, 0, 0, 0, 0, 0, 0, 0, // 1 as i64
            1, // OP_CONSTANT_INT
            2, 0, 0, 0, 0, 0, 0, 0,  // 2 as i64
            7,  // OP_ADD
            35, // OP_HALT
        ];
        let program = create_program(bytecode, vec![]);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }

    #[test]
    fn test_string_concatenation() {
        let bytecode = vec![
            3,  // OP_CONSTANT_STRING
            0,  // index 0
            3,  // OP_CONSTANT_STRING
            1,  // index 1
            7,  // OP_ADD
            35, // OP_HALT
        ];
        let constants = vec![
            Value::String("Hello ".to_string()),
            Value::String("World".to_string()),
        ];
        let program = create_program(bytecode, constants);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }

    #[test]
    fn test_comparison() {
        let bytecode = vec![
            1, // OP_CONSTANT_INT
            5, 0, 0, 0, 0, 0, 0, 0, 1, // OP_CONSTANT_INT
            3, 0, 0, 0, 0, 0, 0, 0, 12, // OP_GREATER
            35, // OP_HALT
        ];
        let program = create_program(bytecode, vec![]);
        let mut vm = VM::new();
        assert!(vm.run(&program).is_ok());
    }
}
