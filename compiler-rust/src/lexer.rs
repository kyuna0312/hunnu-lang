//! Lexer for the Hunnu programming language.
//!
//! Tokenizes Hunnu source code into a stream of [`Token`] values
//! for consumption by the parser.

/// Represents the type of a single token.
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
    Pub,
    SelfToken,
    
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

/// A single token produced by the lexer.
#[derive(Debug, Clone)]
pub struct Token {
    /// The type of this token.
    pub token_type: TokenType,
    /// The raw lexeme string from source.
    pub lexeme: String,
    /// Line number (1-indexed).
    pub line: u32,
    /// Column number (1-indexed).
    pub column: u32,
}

impl Token {
    /// Create a new token.
    pub fn new(token_type: TokenType, lexeme: String, line: u32, column: u32) -> Self {
        Token {
            token_type,
            lexeme,
            line,
            column,
        }
    }
}

/// Hunnu source code lexer.
///
/// Converts a source string into a [`Vec<Token>`] via [`tokenize`](Lexer::tokenize).
pub struct Lexer {
    source: Vec<char>,
    length: usize,
    current: usize,
    line: u32,
    column: u32,
}

impl Lexer {
    /// Create a new lexer for the given source string.
    pub fn new(source: &str) -> Self {
        Lexer {
            source: source.chars().collect(),
            length: source.len(),
            current: 0,
            line: 1,
            column: 1,
        }
    }
    
    /// Tokenize the full source and return all tokens.
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
            "let" | "хувьсагч" => TokenType::Let,
            "fn" | "функц" => TokenType::Fn,
            "if" | "хэрвээ" => TokenType::If,
            "else" | "бусад" => TokenType::Else,
            "true" | "үнэн" => TokenType::True,
            "false" | "худал" => TokenType::False,
            "print" | "хэвлэх" => TokenType::Print,
            "while" | "давталт" => TokenType::While,
            "for" | "тооллого" => TokenType::For,
            "return" | "буцаах" => TokenType::Return,
            "break" | "зогсоох" => TokenType::Break,
            "continue" | "үргэлжлүүлэх" => TokenType::Continue,
            "match" | "тохирох" => TokenType::Match,
            "null" | "nil" | "хоосон" => TokenType::Null,
            "import" | "импорт" => TokenType::Import,
            "extern" | "гаднах" => TokenType::Extern,
            "try" | "турших" => TokenType::Try,
            "catch" | "барих" => TokenType::Catch,
            "finally" | "эцэст" => TokenType::Finally,
            "type" | "төрөл" => TokenType::Type,
            "pub" | "нийт" => TokenType::Pub,
            "self" | "өөрөө" => TokenType::SelfToken,
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

#[cfg(test)]
mod tests {
    use super::*;

    fn tokenize(source: &str) -> Vec<Token> {
        let mut lexer = Lexer::new(source);
        lexer.tokenize()
    }

    #[test]
    fn test_empty_source() {
        let tokens = tokenize("");
        assert_eq!(tokens.len(), 1);
        assert_eq!(tokens[0].token_type, TokenType::EOF);
    }

    #[test]
    fn test_integer_literal() {
        let tokens = tokenize("42");
        assert_eq!(tokens[0].token_type, TokenType::IntLiteral);
        assert_eq!(tokens[0].lexeme, "42");
    }

    #[test]
    fn test_float_literal() {
        let tokens = tokenize("3.14");
        assert_eq!(tokens[0].token_type, TokenType::FloatLiteral);
        assert_eq!(tokens[0].lexeme, "3.14");
    }

    #[test]
    fn test_string_literal() {
        let tokens = tokenize("\"hello\"");
        assert_eq!(tokens[0].token_type, TokenType::StringLiteral);
        assert_eq!(tokens[0].lexeme, "hello");
    }

    #[test]
    fn test_string_escape_newline() {
        let tokens = tokenize("\"a\\nb\"");
        assert_eq!(tokens[0].token_type, TokenType::StringLiteral);
        assert_eq!(tokens[0].lexeme, "a\nb");
    }

    #[test]
    fn test_string_escape_tab() {
        let tokens = tokenize("\"a\\tb\"");
        assert_eq!(tokens[0].token_type, TokenType::StringLiteral);
        assert_eq!(tokens[0].lexeme, "a\tb");
    }

    #[test]
    fn test_string_escape_quote() {
        let tokens = tokenize("\"a\\\"b\"");
        assert_eq!(tokens[0].token_type, TokenType::StringLiteral);
        assert_eq!(tokens[0].lexeme, "a\"b");
    }

    #[test]
    fn test_string_escape_backslash() {
        let tokens = tokenize("\"a\\\\b\"");
        assert_eq!(tokens[0].token_type, TokenType::StringLiteral);
        assert_eq!(tokens[0].lexeme, "a\\b");
    }

    #[test]
    fn test_identifiers() {
        let tokens = tokenize("foo bar _x");
        assert_eq!(tokens.len(), 4);
        assert_eq!(tokens[0].token_type, TokenType::Ident);
        assert_eq!(tokens[0].lexeme, "foo");
        assert_eq!(tokens[1].token_type, TokenType::Ident);
        assert_eq!(tokens[1].lexeme, "bar");
        assert_eq!(tokens[2].token_type, TokenType::Ident);
        assert_eq!(tokens[2].lexeme, "_x");
        assert_eq!(tokens[3].token_type, TokenType::EOF);
    }

    #[test]
    fn test_keywords() {
        let tokens = tokenize("let fn if else true false while for return break continue match print");
        let expected = [
            ("let", TokenType::Let),
            ("fn", TokenType::Fn),
            ("if", TokenType::If),
            ("else", TokenType::Else),
            ("true", TokenType::True),
            ("false", TokenType::False),
            ("while", TokenType::While),
            ("for", TokenType::For),
            ("return", TokenType::Return),
            ("break", TokenType::Break),
            ("continue", TokenType::Continue),
            ("match", TokenType::Match),
            ("print", TokenType::Print),
        ];
        for (i, (lexeme, tt)) in expected.iter().enumerate() {
            assert_eq!(tokens[i].token_type, *tt, "mismatch at index {}: expected {:?} got {:?}",
                       i, tt, tokens[i].token_type);
            assert_eq!(tokens[i].lexeme, *lexeme);
        }
    }

    #[test]
    fn test_operators() {
        let tokens = tokenize("+ - * / % == != < > <= >= =");
        let expected = [
            ("+", TokenType::Plus),
            ("-", TokenType::Minus),
            ("*", TokenType::Star),
            ("/", TokenType::Slash),
            ("%", TokenType::Percent),
            ("==", TokenType::Eq),
            ("!=", TokenType::Neq),
            ("<", TokenType::Lt),
            (">", TokenType::Gt),
            ("<=", TokenType::Le),
            (">=", TokenType::Ge),
            ("=", TokenType::Assign),
        ];
        for (i, (lexeme, tt)) in expected.iter().enumerate() {
            assert_eq!(tokens[i].token_type, *tt, "mismatch at index {}: expected {:?} got {:?}",
                       i, tt, tokens[i].token_type);
            assert_eq!(tokens[i].lexeme, *lexeme);
        }
    }

    #[test]
    fn test_compound_operators() {
        let tokens = tokenize("-> => += -= *= /=");
        assert_eq!(tokens[0].token_type, TokenType::Arrow);
        assert_eq!(tokens[0].lexeme, "->");
        assert_eq!(tokens[1].token_type, TokenType::FatArrow);
        assert_eq!(tokens[1].lexeme, "=>");
        assert_eq!(tokens[2].token_type, TokenType::PlusAssign);
        assert_eq!(tokens[2].lexeme, "+=");
        assert_eq!(tokens[3].token_type, TokenType::MinusAssign);
        assert_eq!(tokens[3].lexeme, "-=");
        assert_eq!(tokens[4].token_type, TokenType::StarAssign);
        assert_eq!(tokens[4].lexeme, "*=");
        assert_eq!(tokens[5].token_type, TokenType::SlashAssign);
        assert_eq!(tokens[5].lexeme, "/=");
    }

    #[test]
    fn test_delimiters() {
        let tokens = tokenize("( ) { } [ ] , . : ; &");
        let expected = [
            ("(", TokenType::LParen),
            (")", TokenType::RParen),
            ("{", TokenType::LBrace),
            ("}", TokenType::RBrace),
            ("[", TokenType::LBracket),
            ("]", TokenType::RBracket),
            (",", TokenType::Comma),
            (".", TokenType::Dot),
            (":", TokenType::Colon),
            (";", TokenType::Semicolon),
            ("&", TokenType::Ampersand),
        ];
        for (i, (lexeme, tt)) in expected.iter().enumerate() {
            assert_eq!(tokens[i].token_type, *tt, "mismatch at index {}", i);
            assert_eq!(tokens[i].lexeme, *lexeme);
        }
    }

    #[test]
    fn test_line_numbers() {
        let tokens = tokenize("a\nb\nc");
        assert_eq!(tokens[0].line, 1);
        assert_eq!(tokens[1].line, 2);
        assert_eq!(tokens[2].line, 3);
    }

    #[test]
    fn test_column_numbers() {
        let tokens = tokenize("  foo");
        assert_eq!(tokens[0].column, 3);
    }

    #[test]
    fn test_comments_skipped() {
        let tokens = tokenize("// comment\n42");
        assert_eq!(tokens.len(), 2);
        assert_eq!(tokens[0].token_type, TokenType::IntLiteral);
        assert_eq!(tokens[0].lexeme, "42");
        assert_eq!(tokens[1].token_type, TokenType::EOF);
    }

    #[test]
    fn test_unknown_character() {
        let tokens = tokenize("@");
        assert_eq!(tokens[0].token_type, TokenType::Unknown);
        assert_eq!(tokens[0].lexeme, "@");
    }

    #[test]
    fn test_mixed_program() {
        let source = "let x = 10\nfn add(a, b) { return a + b }";
        let tokens = tokenize(source);
        let expected_types = [
            TokenType::Let,
            TokenType::Ident,
            TokenType::Assign,
            TokenType::IntLiteral,
            TokenType::Fn,
            TokenType::Ident,
            TokenType::LParen,
            TokenType::Ident,
            TokenType::Comma,
            TokenType::Ident,
            TokenType::RParen,
            TokenType::LBrace,
            TokenType::Return,
            TokenType::Ident,
            TokenType::Plus,
            TokenType::Ident,
            TokenType::RBrace,
            TokenType::EOF,
        ];
        assert_eq!(tokens.len(), expected_types.len(), "token count mismatch");
        for (i, expected) in expected_types.iter().enumerate() {
            assert_eq!(tokens[i].token_type, *expected, "mismatch at token {}", i);
        }
    }

    #[test]
    fn test_logical_operators() {
        let tokens = tokenize("! and or");
        assert_eq!(tokens[0].token_type, TokenType::Not);
        assert_eq!(tokens[0].lexeme, "!");
        assert_eq!(tokens[1].token_type, TokenType::Ident);
        assert_eq!(tokens[1].lexeme, "and");
        assert_eq!(tokens[2].token_type, TokenType::Ident);
        assert_eq!(tokens[2].lexeme, "or");
    }
}

