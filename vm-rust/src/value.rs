/// Value type representing runtime values in Hunnu
/// Matches the C Value struct in compiler/interpreter/interpreter.h
#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    Int(i64),
    Float(f64),
    String(String),
    Bool(bool),
    None,
    Array(Vec<Value>),
}

impl Value {
    pub fn as_bool(&self) -> bool {
        match self {
            Value::Bool(b) => *b,
            Value::Int(n) => *n != 0,
            Value::Float(f) => *f != 0.0,
            Value::String(s) => !s.is_empty(),
            _ => false,
        }
    }

    pub fn print(&self) {
        match self {
            Value::Int(n) => print!("{}", n),
            Value::Float(f) => {
                if f.fract() == 0.0 {
                    print!("{}", f);
                } else {
                    let s = format!("{}", f);
                    print!("{}", s);
                }
            }
            Value::String(s) => print!("{}", s),
            Value::Bool(b) => print!("{}", if *b { "true" } else { "false" }),
            Value::None => print!("nil"),
            Value::Array(arr) => {
                print!("[");
                for (i, v) in arr.iter().enumerate() {
                    if i > 0 {
                        print!(", ");
                    }
                    v.print();
                }
                print!("]");
            }
        }
    }

    pub fn println(&self) {
        self.print();
        println!();
    }
}

impl std::fmt::Display for Value {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Value::Int(n) => write!(f, "{}", n),
            Value::Float(v) => write!(f, "{}", v),
            Value::String(s) => write!(f, "{}", s),
            Value::Bool(b) => write!(f, "{}", if *b { "true" } else { "false" }),
            Value::None => write!(f, "nil"),
            Value::Array(arr) => {
                write!(f, "[")?;
                for (i, v) in arr.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", v)?;
                }
                write!(f, "]")
            }
        }
    }
}
