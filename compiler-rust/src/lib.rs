pub mod lexer;
pub mod parser;
pub mod ast;

#[cfg(feature = "llvm-codegen")]
pub mod codegen;

use std::ffi::{CStr, CString};
use std::os::raw::c_char;

pub use lexer::TokenType;
pub use lexer::Token;
pub use ast::ASTNode;
pub use ast::NodeData;
pub use ast::ASTNodeType;
pub use ast::LiteralType;
pub use ast::LiteralValue;

pub fn lex_source(source: &str) -> Vec<Token> {
    let mut lexer = lexer::Lexer::new(source);
    lexer.tokenize()
}

pub fn parse_source(source: &str) -> Result<ASTNode, String> {
    let tokens = lex_source(source);
    let mut parser = parser::Parser::new(tokens);
    parser.parse()
}

pub fn tokens_to_string(tokens: &[Token]) -> String {
    let mut out = String::new();
    for token in tokens {
        out.push_str(&format!("{:?} '{}' [{}:{}]\n",
            token.token_type, token.lexeme, token.line, token.column));
    }
    out
}

pub fn ast_to_string(node: &ASTNode) -> String {
    format!("{:#?}", node)
}

#[no_mangle]
pub extern "C" fn hunnu_rust_lex(source: *const c_char) -> *mut c_char {
    let c_str = unsafe { CStr::from_ptr(source) };
    let source_str = c_str.to_str().unwrap_or("");
    let tokens = lex_source(source_str);
    let output = tokens_to_string(&tokens);
    CString::new(output).unwrap().into_raw()
}

#[no_mangle]
pub extern "C" fn hunnu_rust_parse(source: *const c_char) -> *mut c_char {
    let c_str = unsafe { CStr::from_ptr(source) };
    let source_str = c_str.to_str().unwrap_or("");
    match parse_source(source_str) {
        Ok(ast) => {
            let output = ast_to_string(&ast);
            CString::new(output).unwrap().into_raw()
        }
        Err(e) => {
            let output = format!("Parse error: {}", e);
            CString::new(output).unwrap().into_raw()
        }
    }
}

#[no_mangle]
pub extern "C" fn hunnu_rust_free_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe { let _ = CString::from_raw(s); }
    }
}
