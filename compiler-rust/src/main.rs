//! Hunnu compiler CLI binary.
//!
//! Accepts a `.hn` source file, tokenizes and parses it,
//! then prints the token stream and AST for debugging.

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

    println!("=== Tokens ===");
    let tokens = hunnu_compiler::lex_source(&source);
    for token in &tokens {
        println!("{:?} '{}' [{}:{}]", token.token_type, token.lexeme, token.line, token.column);
    }

    match hunnu_compiler::parse_source(&source) {
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
