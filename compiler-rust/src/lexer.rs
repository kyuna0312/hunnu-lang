/**
 * @file lexer.rs
 * @brief Rust Lexer for Hunnu Language
 * 
 * This is part of Month 3 AOT Compiler Foundation.
 * The lexer tokenizes Hunnu source code into tokens for the parser.
 */

#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    // Literals
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    BoolLiteral,
    
    // Keywords
    Let,
    Fn,
    If,
    Else,
    True,
    False,
    Print,
    While,
    For,
    Return,
    Break,
    Continue,
    Match,
    Null,
    NilKeyword,
    Import,
    Extern,
    Try,
    Catch,
    Finally,
    Type,
    
    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    
    // Comparison
    Eq,
    Neq,
    Lt,
    Le,
    Gt,
    Ge,
    
    // Assignment
    Assign,
    PlusAssign,
    MinusAssign,
    StarAssign,
    SlashAssign,
    
    // Logical
    And,
    Or,
    Not,
    
    // Special
    Arrow,      // ->
    FatArrow,   // =>
    Newline,
    
    // Punctuation
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Semicolon,
    Comma,
    Colon,
    Dot,
    Ampersand,
    
    // Misc
    Ident,
    EOF,
    Unknown,
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String,
    pub line: u32,
    pub column: u32,
}

impl Token {
    pub fn new(token_type: TokenType, lexeme: String, line: u32, column: u32) -> Self {
        Token {
            token_type,
            lexeme,
            line,
            column,
        }
    }
}

pub struct Lexer {
    source: Vec<char>,
    length: usize,
    current: usize,
    line: u32,
    column: u32,
}

impl Lexer {
    pub fn new(source: &str) -> Self {
        Lexer {
            source: source.chars().collect(),
            length: source.len(),
            current: 0,
            line: 1,
            column: 1,
        }
    }
    
    pub fn tokenize(&mut self) -> Vec<Token> {
        let mut tokens = Vec::new();
        
        while !self.is_at_end() {
            self.skip_whitespace();
            
            if self.is_at_end() {
                break;
            }
            
            if self.check_comment() {
                self.skip_comment();
                continue;
            }
            
            let token = self.next_token();
            tokens.push(token);
        }
        
        tokens.push(Token::new(TokenType::EOF, "".to_string(), self.line, self.column));
        tokens
    }
    
    fn is_at_end(&self) -> bool {
        self.current >= self.length
    }
    
    fn advance(&mut self) -> Option<char> {
        if self.is_at_end() {
            return None;
        }
        
        let c = self.source[self.current];
        self.current += 1;
        
        if c == '\n' {
            self.line += 1;
            self.column = 1;
        } else {
            self.column += 1;
        }
        
        Some(c)
    }
    
    fn peek(&self) -> Option<char> {
        if self.is_at_end() {
            None
        } else {
            Some(self.source[self.current])
        }
    }
    
    fn peek_next(&self) -> Option<char> {
        if self.current + 1 >= self.length {
            None
        } else {
            Some(self.source[self.current + 1])
        }
    }
    
    fn skip_whitespace(&mut self) {
        while let Some(c) = self.peek() {
            if c == ' ' || c == '\t' || c == '\r' || c == '\n' {
                self.advance();
            } else {
                break;
            }
        }
    }
    
    fn check_comment(&self) -> bool {
        if let Some(c) = self.peek() {
            if c == '/' && self.peek_next() == Some('/') {
                return true;
            }
        }
        false
    }
    
    fn skip_comment(&mut self) {
        while !self.is_at_end() && self.peek() != Some('\n') {
            self.advance();
        }
    }
    
    fn next_token(&mut self) -> Token {
        let start_line = self.line;
        let start_column = self.column;
        
        let c = self.advance().unwrap_or('\0');
        
        match c {
            '(' => Token::new(TokenType::LParen, "(".to_string(), start_line, start_column),
            ')' => Token::new(TokenType::RParen, ")".to_string(), start_line, start_column),
            '{' => Token::new(TokenType::LBrace, "{".to_string(), start_line, start_column),
            '}' => Token::new(TokenType::RBrace, "}".to_string(), start_line, start_column),
            '[' => Token::new(TokenType::LBracket, "[".to_string(), start_line, start_column),
            ']' => Token::new(TokenType::RBracket, "]".to_string(), start_line, start_column),
            ';' => Token::new(TokenType::Semicolon, ";".to_string(), start_line, start_column),
            ',' => Token::new(TokenType::Comma, ",".to_string(), start_line, start_column),
            ':' => Token::new(TokenType::Colon, ":".to_string(), start_line, start_column),
            '.' => Token::new(TokenType::Dot, ".".to_string(), start_line, start_column),
            '&' => Token::new(TokenType::Ampersand, "&".to_string(), start_line, start_column),
            
            '+' => {
                if self.match_char('=') {
                    Token::new(TokenType::PlusAssign, "+=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Plus, "+".to_string(), start_line, start_column)
                }
            }
            '-' => {
                if self.match_char('>') {
                    Token::new(TokenType::Arrow, "->".to_string(), start_line, start_column)
                } else if self.match_char('=') {
                    Token::new(TokenType::MinusAssign, "-=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Minus, "-".to_string(), start_line, start_column)
                }
            }
            '*' => {
                if self.match_char('=') {
                    Token::new(TokenType::StarAssign, "*=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Star, "*".to_string(), start_line, start_column)
                }
            }
            '/' => {
                if self.match_char('=') {
                    Token::new(TokenType::SlashAssign, "/=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Slash, "/".to_string(), start_line, start_column)
                }
            }
            '%' => Token::new(TokenType::Percent, "%".to_string(), start_line, start_column),
            
            '=' => {
                if self.match_char('=') {
                    Token::new(TokenType::Eq, "==".to_string(), start_line, start_column)
                } else if self.match_char('>') {
                    Token::new(TokenType::FatArrow, "=>".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Assign, "=".to_string(), start_line, start_column)
                }
            }
            '!' => {
                if self.match_char('=') {
                    Token::new(TokenType::Neq, "!=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Not, "!".to_string(), start_line, start_column)
                }
            }
            '<' => {
                if self.match_char('=') {
                    Token::new(TokenType::Le, "<=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Lt, "<".to_string(), start_line, start_column)
                }
            }
            '>' => {
                if self.match_char('=') {
                    Token::new(TokenType::Ge, ">=".to_string(), start_line, start_column)
                } else {
                    Token::new(TokenType::Gt, ">".to_string(), start_line, start_column)
                }
            }
            
            '"' => self.read_string(start_line, start_column),
            
            c if c.is_alphabetic() || c == '_' => self.read_identifier(c, start_line, start_column),
            c if c.is_digit(10) => self.read_number(c, start_line, start_column),
            
            _ => {
                eprintln!("[{}:{}] Warning: Unknown character '{}'", start_line, start_column, c);
                Token::new(TokenType::Unknown, c.to_string(), start_line, start_column)
            }
        }
    }
    
    fn match_char(&mut self, expected: char) -> bool {
        if self.is_at_end() {
            return false;
        }
        
        if self.source[self.current] != expected {
            return false;
        }
        
        self.advance();
        true
    }
    
    fn read_identifier(&mut self, first: char, line: u32, column: u32) -> Token {
        let mut lexeme = String::new();
        lexeme.push(first);
        
        while let Some(c) = self.peek() {
            if c.is_alphanumeric() || c == '_' {
                lexeme.push(self.advance().unwrap());
            } else {
                break;
            }
        }
        
        let token_type = match lexeme.as_str() {
            "let" => TokenType::Let,
            "fn" => TokenType::Fn,
            "if" => TokenType::If,
            "else" => TokenType::Else,
            "true" => TokenType::True,
            "false" => TokenType::False,
            "print" => TokenType::Print,
            "while" => TokenType::While,
            "for" => TokenType::For,
            "return" => TokenType::Return,
            "break" => TokenType::Break,
            "continue" => TokenType::Continue,
            "match" => TokenType::Match,
            "null" | "nil" => TokenType::Null,
            "import" => TokenType::Import,
            "extern" => TokenType::Extern,
            "try" => TokenType::Try,
            "catch" => TokenType::Catch,
            "finally" => TokenType::Finally,
            "type" => TokenType::Type,
            _ => TokenType::Ident,
        };
        
        Token::new(token_type, lexeme, line, column)
    }
    
    fn read_number(&mut self, first: char, line: u32, column: u32) -> Token {
        let mut lexeme = String::new();
        lexeme.push(first);
        let mut is_float = false;
        
        while let Some(c) = self.peek() {
            if c.is_digit(10) {
                lexeme.push(self.advance().unwrap());
            } else if c == '.' && !is_float {
                is_float = true;
                lexeme.push(self.advance().unwrap());
            } else {
                break;
            }
        }
        
        let token_type = if is_float {
            TokenType::FloatLiteral
        } else {
            TokenType::IntLiteral
        };
        
        Token::new(token_type, lexeme, line, column)
    }
    
    fn read_string(&mut self, line: u32, column: u32) -> Token {
        let mut value = String::new();
        
        while let Some(c) = self.peek() {
            if c == '"' {
                self.advance();
                break;
            }
            
            if c == '\\' {
                self.advance();
                if let Some(escaped) = self.peek() {
                    match escaped {
                        'n' => {
                            self.advance();
                            value.push('\n');
                        }
                        't' => {
                            self.advance();
                            value.push('\t');
                        }
                        '\\' => {
                            self.advance();
                            value.push('\\');
                        }
                        '"' => {
                            self.advance();
                            value.push('"');
                        }
                        _ => {
                            value.push('\\');
                        }
                    }
                }
            } else {
                value.push(self.advance().unwrap());
            }
        }
        
        Token::new(TokenType::StringLiteral, value, line, column)
    }
}
