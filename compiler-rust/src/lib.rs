/**
 * @file lib.rs
 * @brief Hunnu Language Rust Compiler - Month 3 AOT Compiler Foundation
 * 
 * This is the main entry point for the Rust-based compiler
 * that will eventually replace the C implementation.
 */

pub mod lexer;
pub mod parser;
pub mod ast;
pub mod codegen;

use std::env;
use std::fs;
use std::process;

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        eprintln!("Usage: {} <source.hn>", args[0]);
        process::exit(1);
    }
    
    let filename = &args[1];
    let source = match fs::read_to_string(filename) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Error reading file '{}': {}", filename, e);
            process::exit(1);
        }
    };
    
    // Lexical analysis
    let mut lexer = lexer::Lexer::new(&source);
    let tokens = lexer.tokenize();
    
    println!("=== Tokens ===");
    for token in &tokens {
        println!("{:?}", token);
    }
    
    // Parsing
    let mut parser = parser::Parser::new(tokens);
    match parser.parse() {
        Ok(program) => {
            println!("\n=== AST ===");
            println!("{:#?}", program);
        }
        Err(e) => {
            eprintln!("Parse error: {}", e);
            process::exit(1);
        }
    }
}
