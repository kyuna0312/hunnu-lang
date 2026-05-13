//! Recursive descent parser for the Hunnu programming language.
//!
//! Converts a token stream (from [`crate::lexer`]) into an
//! Abstract Syntax Tree ([`ASTNode`]).

use super::lexer::TokenType;
use super::ast::*;
use crate::lexer::Token;

/// Hunnu recursive descent parser.
pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
    had_error: bool,
}

impl Parser {
    /// Create a new parser from a token vector.
    pub fn new(tokens: Vec<Token>) -> Self {
        Parser {
            tokens,
            current: 0,
            had_error: false,
        }
    }
    
    fn is_at_end(&self) -> bool {
        self.current >= self.tokens.len() ||
        self.tokens[self.current].token_type == TokenType::EOF
    }
    
    fn advance(&mut self) -> Option<Token> {
        if self.is_at_end() {
            None
        } else {
            let token = self.tokens[self.current].clone();
            self.current += 1;
            Some(token)
        }
    }
    
    fn peek(&self) -> Option<&Token> {
        if self.is_at_end() {
            None
        } else {
            Some(&self.tokens[self.current])
        }
    }
    
    fn previous(&self) -> Option<&Token> {
        if self.current == 0 {
            None
        } else {
            Some(&self.tokens[self.current - 1])
        }
    }
    
    fn check(&self, token_type: TokenType) -> bool {
        if let Some(token) = self.peek() {
            token.token_type == token_type
        } else {
            false
        }
    }
    
    fn match_token(&mut self, token_type: TokenType) -> bool {
        if self.check(token_type) {
            self.advance();
            true
        } else {
            false
        }
    }
    
    fn consume(&mut self, token_type: TokenType, message: &str) {
        if self.match_token(token_type) {
            return;
        }
        self.error(message);
    }
    
    fn error(&mut self, message: &str) {
        if self.had_error {
            return;
        }
        self.had_error = true;
        
        if let Some(token) = self.previous() {
            eprintln!("[{}:{}] Error: {} (got '{}')",
                     token.line, token.column, message, token.lexeme);
        } else if let Some(token) = self.peek() {
            eprintln!("[{}:{}] Error: {} (got '{}')",
                     token.line, token.column, message, token.lexeme);
        } else {
            eprintln!("Error: {}", message);
        }
    }
    
    /// Parse the full token stream into an AST.
    pub fn parse(&mut self) -> Result<ASTNode, String> {
        self.parse_program()
    }
    
    fn parse_program(&mut self) -> Result<ASTNode, String> {
        let mut statements = Vec::new();
        
        while !self.is_at_end() {
            let decl = self.parse_declaration()?;
            statements.push(decl);
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::Program,
            line: 1,
            column: 1,
            data: NodeData::Program { statements },
        })
    }
    
    fn parse_declaration(&mut self) -> Result<ASTNode, String> {
        if self.match_token(TokenType::Let) {
            return self.parse_var_decl();
        }
        
        if self.match_token(TokenType::Fn) {
            return self.parse_fn_decl();
        }
        
        if self.match_token(TokenType::Extern) {
            return self.parse_extern_fn();
        }
        
        if self.match_token(TokenType::Type) {
            return self.parse_type_decl();
        }
        
        self.parse_statement()
    }
    
    fn parse_var_decl(&mut self) -> Result<ASTNode, String> {
        let name = if let Some(token) = self.advance() {
            if token.token_type == TokenType::Ident {
                token.lexeme.clone()
            } else {
                self.error("Expected variable name after 'let'");
                return Err("Parse error".to_string());
            }
        } else {
            self.error("Expected variable name after 'let'");
            return Err("Parse error".to_string());
        };
        
        self.consume(TokenType::Assign, "Expected '=' after variable name");
        
        let value = self.parse_expression()?;
        
        if self.check(TokenType::Semicolon) {
            self.advance();
        }
        
        let previous = self.previous().unwrap();
        Ok(ASTNode {
            node_type: ASTNodeType::VarDecl,
            line: previous.line,
            column: previous.column,
            data: NodeData::VarDecl {
                name,
                initializer: Box::new(value),
            },
        })
    }
    
    fn parse_fn_decl(&mut self) -> Result<ASTNode, String> {
        let name = if let Some(token) = self.advance() {
            if token.token_type == TokenType::Ident {
                token.lexeme.clone()
            } else {
                self.error("Expected function name after 'fn'");
                return Err("Parse error".to_string());
            }
        } else {
            self.error("Expected function name after 'fn'");
            return Err("Parse error".to_string());
        };
        
        let mut params = Vec::new();
        
        if self.match_token(TokenType::LParen) {
            while !self.check(TokenType::RParen) {
                if params.len() > 0 {
                    self.consume(TokenType::Comma, "Expected ',' between parameters");
                }
                
                if !self.check(TokenType::Ident) && !self.check(TokenType::SelfToken) {
                    self.error("Expected parameter name");
                    return Err("Parse error".to_string());
                }
                
                if let Some(token) = self.advance() {
                    params.push(token.lexeme.clone());
                }
            }
            self.consume(TokenType::RParen, "Expected ')' after parameters");
        }
        
        let body = self.parse_block()?;
        
        Ok(ASTNode {
            node_type: ASTNodeType::FnDecl,
            line: 1,
            column: 1,
            data: NodeData::FnDecl {
                name,
                params,
                body: Box::new(body),
            },
        })
    }
    
    fn parse_extern_fn(&mut self) -> Result<ASTNode, String> {
        self.consume(TokenType::Fn, "Expected 'fn' after 'extern'");
        
        let name = if let Some(token) = self.advance() {
            if token.token_type == TokenType::Ident {
                token.lexeme.clone()
            } else {
                self.error("Expected function name after 'extern fn'");
                return Err("Parse error".to_string());
            }
        } else {
            self.error("Expected function name after 'extern fn'");
            return Err("Parse error".to_string());
        };
        
        let mut params = Vec::new();
        
        if self.match_token(TokenType::LParen) {
            while !self.check(TokenType::RParen) {
                if params.len() > 0 {
                    self.consume(TokenType::Comma, "Expected ',' between parameters");
                }
                
                if !self.check(TokenType::Ident) && !self.check(TokenType::SelfToken) {
                    self.error("Expected parameter name");
                    return Err("Parse error".to_string());
                }
                
                if let Some(token) = self.advance() {
                    params.push(token.lexeme.clone());
                }
            }
            self.consume(TokenType::RParen, "Expected ')' after parameters");
        }
        
        let mut returns_int = 1;
        if self.match_token(TokenType::Arrow) {
            if let Some(token) = self.peek() {
                if token.lexeme == "int" {
                    returns_int = 1;
                    self.advance();
                } else if token.lexeme == "void" {
                    returns_int = 0;
                    self.advance();
                } else if token.lexeme == "str" {
                    returns_int = 2;
                    self.advance();
                } else if token.lexeme == "float" {
                    returns_int = 3;
                    self.advance();
                }
            }
        }
        
        let mut lib_name = None;
        if let Some(token) = self.peek() {
            if token.lexeme == "from" {
                self.advance();
                if let Some(lib_token) = self.advance() {
                    if lib_token.token_type == TokenType::StringLiteral {
                        lib_name = Some(lib_token.lexeme.clone());
                    }
                }
            }
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::ExternFn,
            line: 1,
            column: 1,
            data: NodeData::ExternFn {
                name: name.clone(),
                lib_name,
                symbol_name: name,
                params,
                returns_int,
            },
        })
    }
    
    fn parse_type_decl(&mut self) -> Result<ASTNode, String> {
        let name = if let Some(token) = self.advance() {
            if token.token_type == TokenType::Ident {
                token.lexeme.clone()
            } else {
                self.error("Expected type name after 'type'");
                return Err("Parse error".to_string());
            }
        } else {
            self.error("Expected type name after 'type'");
            return Err("Parse error".to_string());
        };
        
        self.consume(TokenType::Assign, "Expected '=' after type name");
        self.consume(TokenType::LBrace, "Expected '{' after '='");
        
        let mut fields = Vec::new();
        
        while !self.check(TokenType::RBrace) && !self.is_at_end() {
            if fields.len() > 0 {
                if self.check(TokenType::Comma) {
                    self.advance();
                }
            }
            
            if self.check(TokenType::Pub) {
                self.advance();
            }

            if !self.check(TokenType::Ident) && !self.check(TokenType::SelfToken) {
                self.error("Expected field name");
                break;
            }
            
            if let Some(token) = self.advance() {
                fields.push(token.lexeme.clone());
            }
            
            if self.check(TokenType::Colon) {
                self.advance();
                if self.check(TokenType::Ident) {
                    self.advance();
                }
            }
        }
        
        self.consume(TokenType::RBrace, "Expected '}' after type fields");
        
        Ok(ASTNode {
            node_type: ASTNodeType::TypeDecl,
            line: 1,
            column: 1,
            data: NodeData::TypeDecl { name, fields },
        })
    }
    
    fn parse_statement(&mut self) -> Result<ASTNode, String> {
        if self.match_token(TokenType::If) {
            return self.parse_if_statement();
        }
        
        if self.match_token(TokenType::While) {
            return self.parse_while_statement();
        }
        
        if self.match_token(TokenType::For) {
            return self.parse_for_statement();
        }
        
        if self.match_token(TokenType::Return) {
            return self.parse_return_statement();
        }
        
        if self.match_token(TokenType::Break) {
            return Ok(ASTNode {
                node_type: ASTNodeType::BreakStmt,
                line: self.previous().unwrap().line,
                column: self.previous().unwrap().column,
                data: NodeData::BreakStmt,
            });
        }
        
        if self.match_token(TokenType::Continue) {
            return Ok(ASTNode {
                node_type: ASTNodeType::ContinueStmt,
                line: self.previous().unwrap().line,
                column: self.previous().unwrap().column,
                data: NodeData::ContinueStmt,
            });
        }
        
        if self.match_token(TokenType::Try) {
            return self.parse_try_statement();
        }
        
        if self.match_token(TokenType::Print) {
            return self.parse_print_statement();
        }
        
        if self.check(TokenType::LBrace) {
            return self.parse_block();
        }
        
        self.parse_expression_statement()
    }
    
    fn parse_block(&mut self) -> Result<ASTNode, String> {
        self.consume(TokenType::LBrace, "Expected '{'");
        
        let mut statements = Vec::new();
        
        while !self.check(TokenType::RBrace) && !self.is_at_end() {
            let stmt = self.parse_declaration()?;
            statements.push(stmt);
        }
        
        self.consume(TokenType::RBrace, "Expected '}' after block");
        
        let previous = self.previous().unwrap();
        Ok(ASTNode {
            node_type: ASTNodeType::Block,
            line: previous.line,
            column: previous.column,
            data: NodeData::Block { statements },
        })
    }
    
    fn parse_if_statement(&mut self) -> Result<ASTNode, String> {
        let condition = self.parse_expression()?;
        let then_branch = self.parse_statement()?;
        
        let mut else_branch = None;
        if self.match_token(TokenType::Else) {
            if self.match_token(TokenType::If) {
                else_branch = Some(Box::new(self.parse_if_statement()?));
            } else {
                else_branch = Some(Box::new(self.parse_statement()?));
            }
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::IfStmt,
            line: 1,
            column: 1,
            data: NodeData::IfStmt {
                condition: Box::new(condition),
                then_branch: Box::new(then_branch),
                else_branch,
            },
        })
    }
    
    fn parse_while_statement(&mut self) -> Result<ASTNode, String> {
        self.consume(TokenType::LParen, "Expected '(' after 'while'");
        let condition = self.parse_expression()?;
        self.consume(TokenType::RParen, "Expected ')' after condition");
        
        let body = self.parse_statement()?;
        
        Ok(ASTNode {
            node_type: ASTNodeType::WhileStmt,
            line: 1,
            column: 1,
            data: NodeData::WhileStmt {
                condition: Box::new(condition),
                body: Box::new(body),
            },
        })
    }
    
    fn parse_for_statement(&mut self) -> Result<ASTNode, String> {
        let has_parens = self.match_token(TokenType::LParen);
        
        let mut initializer = None;
        if !self.check(TokenType::Semicolon) {
            initializer = Some(Box::new(self.parse_declaration()?));
            if self.check(TokenType::Semicolon) {
                self.advance();
            }
        } else {
            self.consume(TokenType::Semicolon, "Expected ';'");
        }
        
        let mut condition = None;
        if !self.check(TokenType::Semicolon) {
            condition = Some(Box::new(self.parse_expression()?));
        }
        self.consume(TokenType::Semicolon, "Expected ';' after loop condition");
        
        let mut update = None;
        if !self.check(TokenType::RParen) && !self.check(TokenType::LBrace) {
            update = Some(Box::new(self.parse_expression()?));
        }
        
        if has_parens {
            self.consume(TokenType::RParen, "Expected ')' after for clauses");
        }
        
        let body = self.parse_statement()?;
        
        Ok(ASTNode {
            node_type: ASTNodeType::ForStmt,
            line: 1,
            column: 1,
            data: NodeData::ForStmt {
                initializer,
                condition,
                update,
                body: Box::new(body),
            },
        })
    }
    
    fn parse_return_statement(&mut self) -> Result<ASTNode, String> {
        let mut value = None;
        if !self.check(TokenType::RBrace) && !self.check(TokenType::EOF) {
            value = Some(Box::new(self.parse_expression()?));
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::ReturnStmt,
            line: 1,
            column: 1,
            data: NodeData::ReturnStmt { value },
        })
    }
    
    fn parse_print_statement(&mut self) -> Result<ASTNode, String> {
        self.consume(TokenType::LParen, "Expected '(' after 'print'");
        let argument = self.parse_expression()?;
        self.consume(TokenType::RParen, "Expected ')' after argument");
        
        if self.check(TokenType::Semicolon) {
            self.advance();
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::PrintStmt,
            line: 1,
            column: 1,
            data: NodeData::PrintStmt {
                argument: Box::new(argument),
            },
        })
    }
    
    fn parse_try_statement(&mut self) -> Result<ASTNode, String> {
        let try_block = self.parse_statement()?;
        
        let mut catch_var = None;
        let mut catch_block = None;
        
        if self.match_token(TokenType::Catch) {
            if self.check(TokenType::LParen) {
                self.advance();
                if self.check(TokenType::Ident) {
                    if let Some(token) = self.advance() {
                        catch_var = Some(token.lexeme.clone());
                    }
                }
                self.consume(TokenType::RParen, "Expected ')' after catch variable");
            } else if self.check(TokenType::Ident) {
                if let Some(token) = self.advance() {
                    catch_var = Some(token.lexeme.clone());
                }
            }
            
            catch_block = Some(Box::new(self.parse_statement()?));
        }
        
        let mut finally_block = None;
        if self.match_token(TokenType::Finally) {
            finally_block = Some(Box::new(self.parse_statement()?));
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::TryStmt,
            line: 1,
            column: 1,
            data: NodeData::TryStmt {
                try_block: Box::new(try_block),
                catch_var,
                catch_block,
                finally_block,
            },
        })
    }
    
    fn parse_expression_statement(&mut self) -> Result<ASTNode, String> {
        let expr = self.parse_expression()?;
        
        if self.check(TokenType::Semicolon) {
            self.advance();
        }
        
        Ok(ASTNode {
            node_type: ASTNodeType::ExprStmt,
            line: 1,
            column: 1,
            data: NodeData::ExprStmt {
                expression: Box::new(expr),
            },
        })
    }
    
    fn parse_expression(&mut self) -> Result<ASTNode, String> {
        self.parse_assignment()
    }
    
    fn parse_assignment(&mut self) -> Result<ASTNode, String> {
        let left = self.parse_equality()?;
        
        if self.match_token(TokenType::Assign) {
            if let ASTNodeType::Identifier = left.node_type {
                let name = if let NodeData::Identifier { name } = &left.data {
                    name.clone()
                } else {
                    "".to_string()
                };
                let value = self.parse_assignment()?;
                return Ok(ASTNode {
                    node_type: ASTNodeType::Assign,
                    line: left.line,
                    column: left.column,
                    data: NodeData::Assign {
                        name,
                        value: Box::new(value),
                    },
                });
            }
        }
        
        Ok(left)
    }
    
    fn parse_equality(&mut self) -> Result<ASTNode, String> {
        let mut left = self.parse_comparison()?;
        
        while self.match_token(TokenType::Eq) || self.match_token(TokenType::Neq) {
            let op = self.previous().unwrap().token_type.clone();
            let right = self.parse_comparison()?;
            left = ASTNode {
                node_type: ASTNodeType::BinaryExpr,
                line: left.line,
                column: left.column,
                data: NodeData::BinaryExpr {
                    operator: op,
                    left: Box::new(left),
                    right: Box::new(right),
                },
            };
        }
        
        Ok(left)
    }
    
    fn parse_comparison(&mut self) -> Result<ASTNode, String> {
        let mut left = self.parse_addition()?;
        
        while self.match_token(TokenType::Gt) || self.match_token(TokenType::Ge) ||
              self.match_token(TokenType::Lt) || self.match_token(TokenType::Le) {
            let op = self.previous().unwrap().token_type.clone();
            let right = self.parse_addition()?;
            left = ASTNode {
                node_type: ASTNodeType::BinaryExpr,
                line: left.line,
                column: left.column,
                data: NodeData::BinaryExpr {
                    operator: op,
                    left: Box::new(left),
                    right: Box::new(right),
                },
            };
        }
        
        Ok(left)
    }
    
    fn parse_addition(&mut self) -> Result<ASTNode, String> {
        let mut left = self.parse_multiplication()?;
        
        while self.match_token(TokenType::Plus) || self.match_token(TokenType::Minus) {
            let op = self.previous().unwrap().token_type.clone();
            let right = self.parse_multiplication()?;
            left = ASTNode {
                node_type: ASTNodeType::BinaryExpr,
                line: left.line,
                column: left.column,
                data: NodeData::BinaryExpr {
                    operator: op,
                    left: Box::new(left),
                    right: Box::new(right),
                },
            };
        }
        
        Ok(left)
    }
    
    fn parse_multiplication(&mut self) -> Result<ASTNode, String> {
        let mut left = self.parse_unary()?;
        
        while self.match_token(TokenType::Star) || self.match_token(TokenType::Slash) ||
              self.match_token(TokenType::Percent) {
            let op = self.previous().unwrap().token_type.clone();
            let right = self.parse_unary()?;
            left = ASTNode {
                node_type: ASTNodeType::BinaryExpr,
                line: left.line,
                column: left.column,
                data: NodeData::BinaryExpr {
                    operator: op,
                    left: Box::new(left),
                    right: Box::new(right),
                },
            };
        }
        
        Ok(left)
    }
    
    fn parse_unary(&mut self) -> Result<ASTNode, String> {
        if self.match_token(TokenType::Minus) {
            let op = self.previous().unwrap().token_type.clone();
            let operand = self.parse_unary()?;
            return Ok(ASTNode {
                node_type: ASTNodeType::UnaryExpr,
                line: 1,
                column: 1,
                data: NodeData::UnaryExpr {
                    operator: op,
                    operand: Box::new(operand),
                },
            });
        }
        
        if self.match_token(TokenType::Not) {
            let op = self.previous().unwrap().token_type.clone();
            let operand = self.parse_unary()?;
            return Ok(ASTNode {
                node_type: ASTNodeType::UnaryExpr,
                line: 1,
                column: 1,
                data: NodeData::UnaryExpr {
                    operator: op,
                    operand: Box::new(operand),
                },
            });
        }
        
        if self.match_token(TokenType::Ampersand) {
            let operand = self.parse_unary()?;
            return Ok(ASTNode {
                node_type: ASTNodeType::AddressOf,
                line: 1,
                column: 1,
                data: NodeData::AddressOf {
                    operand: Box::new(operand),
                },
            });
        }
        
        if self.match_token(TokenType::Star) {
            let operand = self.parse_unary()?;
            return Ok(ASTNode {
                node_type: ASTNodeType::Dereference,
                line: 1,
                column: 1,
                data: NodeData::Dereference {
                    operand: Box::new(operand),
                },
            });
        }
        
        self.parse_postfix()
    }
    
    fn parse_postfix(&mut self) -> Result<ASTNode, String> {
        let mut expr = self.parse_primary()?;
        
        while self.match_token(TokenType::Dot) {
            if !self.check(TokenType::Ident) {
                self.error("Expected field name after '.'");
                return Err("Parse error".to_string());
            }
            
            let field = if let Some(token) = self.advance() {
                token.lexeme.clone()
            } else {
                "".to_string()
            };
            
            expr = ASTNode {
                node_type: ASTNodeType::FieldAccess,
                line: 1,
                column: 1,
                data: NodeData::FieldAccess {
                    object: Box::new(expr),
                    field,
                },
            };
        }
        
        Ok(expr)
    }
    
    fn parse_primary(&mut self) -> Result<ASTNode, String> {
        if self.match_token(TokenType::IntLiteral) {
            let token = self.previous().unwrap();
            let value = token.lexeme.parse::<i64>().unwrap_or(0);
            return Ok(ASTNode {
                node_type: ASTNodeType::Literal,
                line: token.line,
                column: token.column,
                data: NodeData::Literal {
                    literal_type: LiteralType::Int,
                    value: LiteralValue::Int(value),
                },
            });
        }
        
        if self.match_token(TokenType::FloatLiteral) {
            let token = self.previous().unwrap();
            let value = token.lexeme.parse::<f64>().unwrap_or(0.0);
            return Ok(ASTNode {
                node_type: ASTNodeType::Literal,
                line: token.line,
                column: token.column,
                data: NodeData::Literal {
                    literal_type: LiteralType::Float,
                    value: LiteralValue::Float(value),
                },
            });
        }
        
        if self.match_token(TokenType::StringLiteral) {
            let token = self.previous().unwrap();
            return Ok(ASTNode {
                node_type: ASTNodeType::Literal,
                line: token.line,
                column: token.column,
                data: NodeData::Literal {
                    literal_type: LiteralType::String,
                    value: LiteralValue::String(token.lexeme.clone()),
                },
            });
        }
        
        if self.match_token(TokenType::True) {
            return Ok(ASTNode {
                node_type: ASTNodeType::Literal,
                line: 1,
                column: 1,
                data: NodeData::Literal {
                    literal_type: LiteralType::Bool,
                    value: LiteralValue::Bool(true),
                },
            });
        }
        
        if self.match_token(TokenType::False) {
            return Ok(ASTNode {
                node_type: ASTNodeType::Literal,
                line: 1,
                column: 1,
                data: NodeData::Literal {
                    literal_type: LiteralType::Bool,
                    value: LiteralValue::Bool(false),
                },
            });
        }
        
        if self.match_token(TokenType::Null) || self.match_token(TokenType::NilKeyword) {
            return Ok(ASTNode {
                node_type: ASTNodeType::Literal,
                line: 1,
                column: 1,
                data: NodeData::Literal {
                    literal_type: LiteralType::Int,
                    value: LiteralValue::Int(0),
                },
            });
        }
        
        if self.match_token(TokenType::Ident) || self.match_token(TokenType::SelfToken) {
            let token = self.previous().cloned().unwrap();
            let name = token.lexeme.clone();
            let line = token.line;
            let column = token.column;
            
            if self.match_token(TokenType::LParen) {
                let mut args = Vec::new();
                
                if !self.check(TokenType::RParen) {
                    args.push(self.parse_expression()?);
                    while self.match_token(TokenType::Comma) {
                        args.push(self.parse_expression()?);
                    }
                }
                
                self.consume(TokenType::RParen, "Expected ')' after function arguments");
                
                return Ok(ASTNode {
                    node_type: ASTNodeType::CallExpr,
                    line,
                    column,
                    data: NodeData::CallExpr { name, args },
                });
            }
            
            if self.match_token(TokenType::LBracket) {
                let index = self.parse_expression()?;
                self.consume(TokenType::RBracket, "Expected ']' after index");
                
                return Ok(ASTNode {
                    node_type: ASTNodeType::IndexExpr,
                    line,
                    column,
                    data: NodeData::IndexExpr {
                        array: Box::new(ASTNode {
                            node_type: ASTNodeType::Identifier,
                            line,
                            column,
                            data: NodeData::Identifier { name: name.clone() },
                        }),
                        index: Box::new(index),
                    },
                });
            }
            
            return Ok(ASTNode {
                node_type: ASTNodeType::Identifier,
                line,
                column,
                data: NodeData::Identifier { name },
            });
        }
        
        if self.match_token(TokenType::LParen) {
            let expr = self.parse_expression()?;
            self.consume(TokenType::RParen, "Expected ')' after expression");
            return Ok(expr);
        }
        
        if self.match_token(TokenType::LBracket) {
            let mut elements = Vec::new();
            
            if !self.check(TokenType::RBracket) {
                elements.push(self.parse_expression()?);
                while self.match_token(TokenType::Comma) {
                    elements.push(self.parse_expression()?);
                }
            }
            
            self.consume(TokenType::RBracket, "Expected ']' after array elements");
            
            return Ok(ASTNode {
                node_type: ASTNodeType::ArrayExpr,
                line: 1,
                column: 1,
                data: NodeData::ArrayExpr { elements },
            });
        }
        
        self.error("Expected expression");
        Err("Parse error".to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;

    fn parse(source: &str) -> Result<ASTNode, String> {
        let tokens = {
            let mut lexer = Lexer::new(source);
            lexer.tokenize()
        };
        let mut parser = Parser::new(tokens);
        parser.parse()
    }

    #[test]
    fn test_parse_var_decl() {
        let ast = parse("let x = 42").unwrap();
        assert_eq!(ast.node_type, ASTNodeType::Program);
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements.len(), 1);
            assert_eq!(statements[0].node_type, ASTNodeType::VarDecl);
            if let NodeData::VarDecl { name, initializer } = &statements[0].data {
                assert_eq!(name, "x");
                assert_eq!(initializer.node_type, ASTNodeType::Literal);
            } else {
                panic!("Expected VarDecl");
            }
        } else {
            panic!("Expected Program");
        }
    }

    #[test]
    fn test_parse_fn_decl() {
        let ast = parse("fn add(a, b) { return a + b }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::FnDecl);
            if let NodeData::FnDecl { name, params, body } = &statements[0].data {
                assert_eq!(name, "add");
                assert_eq!(params.len(), 2);
                assert_eq!(params[0], "a");
                assert_eq!(params[1], "b");
                assert_eq!(body.node_type, ASTNodeType::Block);
            } else {
                panic!("Expected FnDecl");
            }
        }
    }

    #[test]
    fn test_parse_if_else() {
        let ast = parse("if x > 0 { print(1) } else { print(2) }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::IfStmt);
            if let NodeData::IfStmt { condition, then_branch, else_branch } = &statements[0].data {
                assert_eq!(condition.node_type, ASTNodeType::BinaryExpr);
                assert_eq!(then_branch.node_type, ASTNodeType::Block);
                assert!(else_branch.is_some());
            } else {
                panic!("Expected IfStmt");
            }
        }
    }

    #[test]
    fn test_parse_while() {
        let ast = parse("while (x < 10) { x = x + 1 }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::WhileStmt);
        }
    }

    #[test]
    fn test_parse_for() {
        let ast = parse("for (let i = 0; i < 10; i = i + 1) { print(i) }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::ForStmt);
            if let NodeData::ForStmt { initializer, condition, update, body } = &statements[0].data {
                assert!(initializer.is_some());
                assert!(condition.is_some());
                assert!(update.is_some());
                assert_eq!(body.node_type, ASTNodeType::Block);
            } else {
                panic!("Expected ForStmt");
            }
        }
    }

    #[test]
    fn test_parse_return() {
        let ast = parse("fn f() { return 42 }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::FnDecl { body, .. } = &statements[0].data {
                if let NodeData::Block { statements } = &body.data {
                    assert_eq!(statements[0].node_type, ASTNodeType::ReturnStmt);
                }
            }
        }
    }

    #[test]
    fn test_parse_break_continue() {
        let ast = parse("fn f() { while true { break } }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::FnDecl { body, .. } = &statements[0].data {
                if let NodeData::Block { statements } = &body.data {
                    assert_eq!(statements[0].node_type, ASTNodeType::WhileStmt);
                    if let NodeData::WhileStmt { body, .. } = &statements[0].data {
                        if let NodeData::Block { statements } = &body.data {
                            assert_eq!(statements[0].node_type, ASTNodeType::BreakStmt);
                        }
                    }
                }
            }
        }
    }

    #[test]
    fn test_parse_print() {
        let ast = parse("print(42)").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::PrintStmt);
        }
    }

    #[test]
    fn test_parse_binary_expr_precedence() {
        let ast = parse("let x = 3 + 4 * 5").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::BinaryExpr);
                if let NodeData::BinaryExpr { operator, left, right } = &initializer.data {
                    assert_eq!(*operator, TokenType::Plus);
                    assert_eq!(right.node_type, ASTNodeType::BinaryExpr);
                    if let NodeData::BinaryExpr { operator, .. } = &right.data {
                        assert_eq!(*operator, TokenType::Star);
                    }
                }
            }
        }
    }

    #[test]
    fn test_parse_unary_minus() {
        let ast = parse("let x = -5").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::UnaryExpr);
                if let NodeData::UnaryExpr { operator, .. } = &initializer.data {
                    assert_eq!(*operator, TokenType::Minus);
                }
            }
        }
    }

    #[test]
    fn test_parse_logical_not() {
        let ast = parse("let x = !true").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::UnaryExpr);
                if let NodeData::UnaryExpr { operator, .. } = &initializer.data {
                    assert_eq!(*operator, TokenType::Not);
                }
            }
        }
    }

    #[test]
    fn test_parse_fn_call() {
        let ast = parse("let r = add(3, 4)").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::CallExpr);
                if let NodeData::CallExpr { name, args } = &initializer.data {
                    assert_eq!(name, "add");
                    assert_eq!(args.len(), 2);
                }
            }
        }
    }

    #[test]
    fn test_parse_array() {
        let ast = parse("let arr = [1, 2, 3]").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::ArrayExpr);
                if let NodeData::ArrayExpr { elements } = &initializer.data {
                    assert_eq!(elements.len(), 3);
                }
            }
        }
    }

    #[test]
    fn test_parse_index_expr() {
        let ast = parse("let v = arr[0]").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::IndexExpr);
            }
        }
    }

    #[test]
    fn test_parse_comparison_chain() {
        let ast = parse("let x = a < b && c > d").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::BinaryExpr);
            }
        }
    }

    #[test]
    fn test_parse_equality() {
        let ast = parse("let x = a == b").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::BinaryExpr);
                if let NodeData::BinaryExpr { operator, .. } = &initializer.data {
                    assert_eq!(*operator, TokenType::Eq);
                }
            }
        }
    }

    #[test]
    fn test_parse_inequality() {
        let ast = parse("let x = a != b").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                if let NodeData::BinaryExpr { operator, .. } = &initializer.data {
                    assert_eq!(*operator, TokenType::Neq);
                }
            }
        }
    }

    #[test]
    fn test_parse_grouping() {
        let ast = parse("let x = (1 + 2) * 3").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::BinaryExpr);
                if let NodeData::BinaryExpr { operator, left, .. } = &initializer.data {
                    assert_eq!(*operator, TokenType::Star);
                    assert_eq!(left.node_type, ASTNodeType::BinaryExpr);
                    if let NodeData::BinaryExpr { operator, .. } = &left.data {
                        assert_eq!(*operator, TokenType::Plus);
                    }
                }
            }
        }
    }

    #[test]
    fn test_parse_type_decl() {
        let ast = parse("type Point = { x, y }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::TypeDecl);
            if let NodeData::TypeDecl { name, fields } = &statements[0].data {
                assert_eq!(name, "Point");
                assert_eq!(fields.len(), 2);
                assert_eq!(fields[0], "x");
                assert_eq!(fields[1], "y");
            }
        }
    }

    #[test]
    fn test_parse_extern_fn() {
        let ast = parse("extern fn puts(str) -> int from \"libc.so.6\"").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::ExternFn);
            if let NodeData::ExternFn { name, params, returns_int, lib_name, .. } = &statements[0].data {
                assert_eq!(name, "puts");
                assert_eq!(params.len(), 1);
                assert_eq!(*returns_int, 1);
                assert!(lib_name.is_some());
                assert_eq!(lib_name.as_ref().unwrap(), "libc.so.6");
            }
        }
    }

    #[test]
    fn test_parse_field_access() {
        let ast = parse("let v = obj.field").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::VarDecl { initializer, .. } = &statements[0].data {
                assert_eq!(initializer.node_type, ASTNodeType::FieldAccess);
                if let NodeData::FieldAccess { object, field } = &initializer.data {
                    assert_eq!(field, "field");
                    assert_eq!(object.node_type, ASTNodeType::Identifier);
                }
            }
        }
    }

    #[test]
    fn test_parse_try_catch() {
        let ast = parse("try { print(1) } catch e { print(2) }").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::TryStmt);
            if let NodeData::TryStmt { catch_var, catch_block, .. } = &statements[0].data {
                assert!(catch_var.is_some());
                assert_eq!(catch_var.as_ref().unwrap(), "e");
                assert!(catch_block.is_some());
            }
        }
    }

    #[test]
    fn test_parse_error_missing_var_name() {
        let tokens = {
            let mut lexer = Lexer::new("let = 1");
            lexer.tokenize()
        };
        let mut parser = Parser::new(tokens);
        let result = parser.parse();
        assert!(result.is_err());
    }

    #[test]
    fn test_parse_empty_program() {
        let ast = parse("").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements.len(), 0);
        }
    }

    #[test]
    fn test_parse_multiple_statements() {
        let ast = parse("let x = 1\nlet y = 2\nlet z = x + y").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements.len(), 3);
        }
    }

    #[test]
    fn test_parse_assignment() {
        let ast = parse("x = 42").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            assert_eq!(statements[0].node_type, ASTNodeType::ExprStmt);
            if let NodeData::ExprStmt { expression } = &statements[0].data {
                assert_eq!(expression.node_type, ASTNodeType::Assign);
                if let NodeData::Assign { name, .. } = &expression.data {
                    assert_eq!(name, "x");
                }
            }
        }
    }

    #[test]
    fn test_parse_extern_void_return() {
        let ast = parse("extern fn foo() -> void").unwrap();
        if let NodeData::Program { statements } = &ast.data {
            if let NodeData::ExternFn { returns_int, .. } = &statements[0].data {
                assert_eq!(*returns_int, 0);
            }
        }
    }

    #[test]
    fn test_parse_nested_blocks() {
        let ast = parse("fn f() { { let x = 1; print(x) } }").unwrap();
        assert!(ast.node_type == ASTNodeType::Program);
    }
}

