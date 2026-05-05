use crate::opcodes::OpCode;
use crate::value::Value;
use std::io::Write;

const STACK_MAX: usize = 256;
const MAX_LOCALS: usize = 256;
const MAX_FRAMES: usize = 64;

pub struct Program {
    pub bytecode: Vec<u8>,
    pub constants: Vec<Value>,
}

struct CallFrame {
    ip: usize,
    locals: Vec<Value>,
    return_ip: usize,
}

#[derive(Clone)]
pub struct Function {
    pub name: String,
    pub entry_point: usize,
    pub arg_count: u8,
}

pub struct VM<W: Write> {
    stack: Vec<Value>,
    frames: Vec<CallFrame>,
    functions: Vec<Function>,
    output: W,
}

impl<W: Write> VM<W> {
    pub fn new_with_output(output: W) -> Self {
        VM {
            stack: Vec::with_capacity(STACK_MAX),
            frames: Vec::with_capacity(MAX_FRAMES),
            functions: Vec::new(),
            output,
        }
    }

    pub fn get_output(&self) -> &W {
        &self.output
    }
}

impl VM<Vec<u8>> {
    pub fn new() -> VM<Vec<u8>> {
        VM {
            stack: Vec::with_capacity(STACK_MAX),
            frames: Vec::with_capacity(MAX_FRAMES),
            functions: Vec::new(),
            output: Vec::new(),
        }
    }

    pub fn take_output(&mut self) -> String {
        String::from_utf8(std::mem::take(&mut self.output)).unwrap_or_default()
    }
}

impl<W: Write> VM<W> {
    fn push(&mut self, value: Value) -> Result<(), String> {
        if self.stack.len() >= STACK_MAX {
            return Err(format!("Stack overflow (max {} values)", STACK_MAX));
        }
        self.stack.push(value);
        Ok(())
    }

    fn pop(&mut self) -> Result<Value, String> {
        self.stack
            .pop()
            .ok_or_else(|| "Stack underflow".to_string())
    }

    fn peek(&self) -> Option<&Value> {
        self.stack.last()
    }

    fn read_i64(bytecode: &[u8], ip: &mut usize) -> i64 {
        let mut val: i64 = 0;
        for i in 0..8 {
            val |= (bytecode[*ip + i] as i64) << (i * 8);
        }
        *ip += 8;
        val
    }

    pub fn define_function(&mut self, name: String, entry_point: usize, arg_count: u8) {
        self.functions.push(Function {
            name,
            entry_point,
            arg_count,
        });
    }

    fn find_function(&self, name: &str) -> Option<&Function> {
        self.functions.iter().find(|f| f.name == name)
    }

    pub fn run(&mut self, program: &Program) -> Result<(), String> {
        self.frames.push(CallFrame {
            ip: 0,
            locals: Vec::with_capacity(MAX_LOCALS),
            return_ip: 0,
        });
        self.execute(program)
    }

    fn execute(&mut self, program: &Program) -> Result<(), String> {
        let bytecode = &program.bytecode;

        loop {
            let ip = self.current_ip();
            if ip >= bytecode.len() {
                return Ok(());
            }

            let op = OpCode::from_byte(bytecode[ip]);
            self.increment_ip(1);

            match op {
                Some(OpCode::ConstantInt) => {
                    let val = Self::read_i64(bytecode, &mut self.current_ip_mut());
                    self.push(Value::Int(val))?;
                }

                Some(OpCode::ConstantFloat) => {
                    let bits = Self::read_i64(bytecode, &mut self.current_ip_mut());
                    let val = f64::from_bits(bits as u64);
                    self.push(Value::Float(val))?;
                }

                Some(OpCode::ConstantString) => {
                    let idx = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    let value = program.constants[idx].clone();
                    self.push(value)?;
                }

                Some(OpCode::Nil) => {
                    self.push(Value::None)?;
                }

                Some(OpCode::True) => {
                    self.push(Value::Bool(true))?;
                }

                Some(OpCode::False) => {
                    self.push(Value::Bool(false))?;
                }

                Some(OpCode::Add) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Int(x + y),
                        (Value::Float(x), Value::Float(y)) => Value::Float(x + y),
                        (Value::Int(x), Value::Float(y)) => Value::Float(x as f64 + y),
                        (Value::Float(x), Value::Int(y)) => Value::Float(x + y as f64),
                        (Value::String(mut x), Value::String(y)) => {
                            x.push_str(&y);
                            Value::String(x)
                        }
                        (Value::String(x), y) => Value::String(x + &y.to_string()),
                        (x, Value::String(y)) => Value::String(x.to_string() + &y),
                        _ => Value::None,
                    };
                    self.push(result)?;
                }

                Some(OpCode::Subtract) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Int(x - y),
                        (Value::Float(x), Value::Float(y)) => Value::Float(x - y),
                        (Value::Int(x), Value::Float(y)) => Value::Float(x as f64 - y),
                        (Value::Float(x), Value::Int(y)) => Value::Float(x - y as f64),
                        _ => Value::None,
                    };
                    self.push(result)?;
                }

                Some(OpCode::Multiply) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Int(x * y),
                        (Value::Float(x), Value::Float(y)) => Value::Float(x * y),
                        (Value::Int(x), Value::Float(y)) => Value::Float(x as f64 * y),
                        (Value::Float(x), Value::Int(y)) => Value::Float(x * y as f64),
                        _ => Value::None,
                    };
                    self.push(result)?;
                }

                Some(OpCode::Divide) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => {
                            if y == 0 {
                                let _ = write!(self.output, "Error: Division by zero");
                                Value::None
                            } else {
                                Value::Int(x / y)
                            }
                        }
                        (Value::Float(x), Value::Float(y)) => {
                            if y == 0.0 {
                                let _ = write!(self.output, "Error: Division by zero");
                                Value::None
                            } else {
                                Value::Float(x / y)
                            }
                        }
                        _ => Value::None,
                    };
                    self.push(result)?;
                }

                Some(OpCode::Greater) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Bool(x > y),
                        (Value::Float(x), Value::Float(y)) => Value::Bool(x > y),
                        (Value::Int(x), Value::Float(y)) => Value::Bool((x as f64) > y),
                        (Value::Float(x), Value::Int(y)) => Value::Bool(x > (y as f64)),
                        _ => Value::Bool(false),
                    };
                    self.push(result)?;
                }

                Some(OpCode::Less) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Bool(x < y),
                        (Value::Float(x), Value::Float(y)) => Value::Bool(x < y),
                        (Value::Int(x), Value::Float(y)) => Value::Bool((x as f64) < y),
                        (Value::Float(x), Value::Int(y)) => Value::Bool(x < (y as f64)),
                        _ => Value::Bool(false),
                    };
                    self.push(result)?;
                }

                Some(OpCode::GreaterEqual) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Bool(x >= y),
                        (Value::Float(x), Value::Float(y)) => Value::Bool(x >= y),
                        (Value::Int(x), Value::Float(y)) => Value::Bool((x as f64) >= y),
                        (Value::Float(x), Value::Int(y)) => Value::Bool(x >= (y as f64)),
                        _ => Value::Bool(false),
                    };
                    self.push(result)?;
                }

                Some(OpCode::LessEqual) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Bool(x <= y),
                        (Value::Float(x), Value::Float(y)) => Value::Bool(x <= y),
                        (Value::Int(x), Value::Float(y)) => Value::Bool((x as f64) <= y),
                        (Value::Float(x), Value::Int(y)) => Value::Bool(x <= (y as f64)),
                        _ => Value::Bool(false),
                    };
                    self.push(result)?;
                }

                Some(OpCode::Equal) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Bool(x == y),
                        (Value::Float(x), Value::Float(y)) => Value::Bool(x == y),
                        (Value::String(x), Value::String(y)) => Value::Bool(x == y),
                        (Value::Bool(x), Value::Bool(y)) => Value::Bool(x == y),
                        (Value::Int(x), Value::Float(y)) => Value::Bool((x as f64) == y),
                        (Value::Float(x), Value::Int(y)) => Value::Bool(x == (y as f64)),
                        _ => Value::Bool(false),
                    };
                    self.push(result)?;
                }

                Some(OpCode::NotEqual) => {
                    let b = self.pop()?;
                    let a = self.pop()?;
                    let result = match (a, b) {
                        (Value::Int(x), Value::Int(y)) => Value::Bool(x != y),
                        (Value::Float(x), Value::Float(y)) => Value::Bool(x != y),
                        (Value::String(x), Value::String(y)) => Value::Bool(x != y),
                        (Value::Bool(x), Value::Bool(y)) => Value::Bool(x != y),
                        (Value::Int(x), Value::Float(y)) => Value::Bool((x as f64) != y),
                        (Value::Float(x), Value::Int(y)) => Value::Bool(x != (y as f64)),
                        _ => Value::Bool(false),
                    };
                    self.push(result)?;
                }

                Some(OpCode::Not) => {
                    let v = self.pop()?;
                    let r = Value::Bool(!v.as_bool());
                    self.push(r)?;
                }

                Some(OpCode::Negate) => {
                    let v = self.pop()?;
                    let r = match v {
                        Value::Int(x) => Value::Int(-x),
                        Value::Float(x) => Value::Float(-x),
                        _ => Value::None,
                    };
                    self.push(r)?;
                }

                Some(OpCode::Jump) => {
                    let offset = Self::read_i64(bytecode, &mut self.current_ip_mut());
                    let ip = self.current_ip_mut();
                    *ip = (*ip as i64 + offset) as usize;
                }

                Some(OpCode::JumpIfFalse) => {
                    let offset = Self::read_i64(bytecode, &mut self.current_ip_mut());
                    if let Some(v) = self.peek() {
                        if !v.as_bool() {
                            let ip = self.current_ip_mut();
                            *ip = (*ip as i64 + offset) as usize;
                        }
                    }
                }

                Some(OpCode::JumpIfTrue) => {
                    let offset = Self::read_i64(bytecode, &mut self.current_ip_mut());
                    if let Some(v) = self.peek() {
                        if v.as_bool() {
                            let ip = self.current_ip_mut();
                            *ip = (*ip as i64 + offset) as usize;
                        }
                    }
                }

                Some(OpCode::Pop) => {
                    self.pop()?;
                }

                Some(OpCode::PopN) => {
                    let n = bytecode[self.current_ip()];
                    self.increment_ip(1);
                    for _ in 0..n {
                        self.pop()?;
                    }
                }

                Some(OpCode::GetLocal) => {
                    let idx = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    let locals = self.current_locals();
                    if idx < locals.len() {
                        let val = locals[idx].clone();
                        self.push(val)?;
                    } else {
                        self.push(Value::None)?;
                    }
                }

                Some(OpCode::SetLocal) => {
                    let idx = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    if let Some(v) = self.peek() {
                        let val = v.clone();
                        let locals = self.current_locals();
                        while locals.len() <= idx {
                            locals.push(Value::None);
                        }
                        locals[idx] = val;
                    }
                }

                Some(OpCode::GetGlobal) => {
                    let _idx = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    self.push(Value::None)?;
                }

                Some(OpCode::SetGlobal) => {
                    let _idx = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                }

                Some(OpCode::CreateArray) => {
                    let count = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    let mut elements = Vec::with_capacity(count);
                    for _ in 0..count {
                        elements.push(self.pop()?);
                    }
                    elements.reverse();
                    self.push(Value::Array(elements))?;
                }

                Some(OpCode::GetIndex) => {
                    let index = self.pop()?;
                    let arr = self.pop()?;
                    if let Value::Array(elements) = arr {
                        let idx: i64 = match &index {
                            Value::Float(f) => *f as i64,
                            Value::Int(n) => *n,
                            _ => 0,
                        };
                        let idx = if idx < 0 {
                            (idx + elements.len() as i64) as usize
                        } else {
                            idx as usize
                        };
                        if idx < elements.len() {
                            self.push(elements[idx].clone())?;
                        } else {
                            self.push(Value::None)?;
                        }
                    } else {
                        self.push(Value::None)?;
                    }
                }

                Some(OpCode::SetIndex) => {
                    let val = self.pop()?;
                    let index = self.pop()?;
                    let arr = self.pop()?;
                    if let Value::Array(mut elements) = arr {
                        let idx: i64 = match &index {
                            Value::Float(f) => *f as i64,
                            Value::Int(n) => *n,
                            _ => 0,
                        };
                        let idx = if idx < 0 {
                            (idx + elements.len() as i64) as usize
                        } else {
                            idx as usize
                        };
                        if idx < elements.len() {
                            elements[idx] = val.clone();
                            self.push(Value::Array(elements))?;
                        } else {
                            self.push(Value::None)?;
                        }
                    } else {
                        self.push(Value::None)?;
                    }
                }

                Some(OpCode::CreateString) => {
                    let count = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    let mut result = String::new();
                    for _ in 0..count {
                        if let Value::String(s) = self.pop()? {
                            result = s + &result;
                        }
                    }
                    self.push(Value::String(result))?;
                }

                Some(OpCode::DefineFn) => {
                    let name_idx = bytecode[self.current_ip()] as usize;
                    self.increment_ip(1);
                    let entry_point = Self::read_i64(bytecode, &mut self.current_ip_mut()) as usize;
                    let arg_count = bytecode[self.current_ip()];
                    self.increment_ip(1);

                    if let Value::String(name) = &program.constants[name_idx] {
                        self.define_function(name.clone(), entry_point, arg_count);
                    }
                }

                Some(OpCode::Call) => {
                    let arg_count = bytecode[self.current_ip()];
                    self.increment_ip(1);
                    let fn_name = self.pop()?;

                    if let Value::String(name) = fn_name {
                        let func_info = self
                            .find_function(&name)
                            .map(|f| (f.entry_point, f.arg_count));

                        if let Some((entry_point, arg_count)) = func_info {
                            let return_ip = self.current_ip();

                            let mut locals = Vec::with_capacity(arg_count as usize);
                            for _ in 0..arg_count {
                                if let Ok(val) = self.pop() {
                                    locals.insert(0, val);
                                }
                            }

                            self.frames.push(CallFrame {
                                ip: entry_point,
                                locals,
                                return_ip,
                            });
                        } else {
                            self.call_builtin(&name, arg_count)?;
                            for _ in 0..arg_count {
                                let _ = self.pop();
                            }
                        }
                    }
                }

                Some(OpCode::Return) => {
                    if self.frames.len() <= 1 {
                        return Ok(());
                    }

                    // Get return value from current frame's stack
                    let return_value = self.pop().ok();
                    let frame = self.frames.pop().unwrap();
                    let ip = self.current_ip_mut();
                    *ip = frame.return_ip;

                    // Push return value onto caller's stack
                    if let Some(v) = return_value {
                        let _ = self.push(v);
                    }
                }

                Some(OpCode::Halt) => {
                    return Ok(());
                }

                Some(_) | None => {
                    return Err(format!(
                        "Unknown opcode at position {}",
                        self.current_ip() - 1
                    ));
                }
            }
        }
    }

    fn current_ip(&self) -> usize {
        self.frames.last().unwrap().ip
    }

    fn current_ip_mut(&mut self) -> &mut usize {
        &mut self.frames.last_mut().unwrap().ip
    }

    fn increment_ip(&mut self, delta: usize) {
        self.frames.last_mut().unwrap().ip += delta;
    }

    fn current_locals(&mut self) -> &mut Vec<Value> {
        &mut self.frames.last_mut().unwrap().locals
    }

    fn call_builtin(&mut self, name: &str, _arg_count: u8) -> Result<(), String> {
        match name {
            "print" => {
                let v = self.peek().cloned();
                if let Some(val) = v {
                    let _ = write!(self.output, "{}", val);
                    let _ = writeln!(self.output);
                }
            }
            "input" => {
                let mut buf = String::new();
                std::io::stdin()
                    .read_line(&mut buf)
                    .map_err(|e| e.to_string())?;
                buf.truncate(buf.trim_end().len());
                self.push(Value::String(buf))?;
            }
            "to_int" => {
                let v = self.pop()?;
                let result = match v {
                    Value::String(s) => Value::Int(s.parse().unwrap_or(0)),
                    Value::Float(f) => Value::Int(f as i64),
                    Value::Int(n) => Value::Int(n),
                    Value::Bool(b) => Value::Int(if b { 1 } else { 0 }),
                    _ => Value::Int(0),
                };
                self.push(result)?;
            }
            "to_float" => {
                let v = self.pop()?;
                let result = match v {
                    Value::String(s) => Value::Float(s.parse().unwrap_or(0.0)),
                    Value::Int(n) => Value::Float(n as f64),
                    Value::Float(f) => Value::Float(f),
                    _ => Value::Float(0.0),
                };
                self.push(result)?;
            }
            "to_str" => {
                let v = self.pop()?;
                self.push(Value::String(v.to_string()))?;
            }
            _ => {
                let _ = writeln!(self.output, "Error: Unknown function '{}'", name);
            }
        }
        Ok(())
    }
}
