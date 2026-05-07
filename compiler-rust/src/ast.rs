/**
 * @file ast.rs
 * @brief AST definitions for Hunnu Language Rust Compiler
 * 
 * This module defines the Abstract Syntax Tree structures
 * for the Month 3 AOT Compiler Foundation.
 */

#[derive(Debug, Clone)]
pub enum ASTNodeType {
    Program,
    VarDecl,
    FnDecl,
    Block,
    IfStmt,
    WhileStmt,
    ForStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    PrintStmt,
    ExprStmt,
    BinaryExpr,
    UnaryExpr,
    Literal,
    Identifier,
    CallExpr,
    Assign,
    ArrayExpr,
    IndexExpr,
    IndexAssign,
    StringConcat,
    MatchExpr,
    ExternFn,
    TryStmt,
    TypeDecl,
    FieldAccess,
    AddressOf,
    Dereference,
}

#[derive(Debug, Clone)]
pub enum LiteralType {
    Int,
    Float,
    String,
    Bool,
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: super::lexer::TokenType,
    pub lexeme: String,
    pub line: u32,
    pub column: u32,
}

#[derive(Debug, Clone)]
pub struct ASTNode {
    pub node_type: ASTNodeType,
    pub line: u32,
    pub column: u32,
    pub data: NodeData,
}

#[derive(Debug, Clone)]
pub enum NodeData {
    Program {
        statements: Vec<ASTNode>,
    },
    VarDecl {
        name: String,
        initializer: Box<ASTNode>,
    },
    FnDecl {
        name: String,
        params: Vec<String>,
        body: Box<ASTNode>,
    },
    Block {
        statements: Vec<ASTNode>,
    },
    IfStmt {
        condition: Box<ASTNode>,
        then_branch: Box<ASTNode>,
        else_branch: Option<Box<ASTNode>>,
    },
    WhileStmt {
        condition: Box<ASTNode>,
        body: Box<ASTNode>,
    },
    ForStmt {
        initializer: Option<Box<ASTNode>>,
        condition: Option<Box<ASTNode>>,
        update: Option<Box<ASTNode>>,
        body: Box<ASTNode>,
    },
    ReturnStmt {
        value: Option<Box<ASTNode>>,
    },
    PrintStmt {
        argument: Box<ASTNode>,
    },
    ExprStmt {
        expression: Box<ASTNode>,
    },
    BinaryExpr {
        operator: super::lexer::TokenType,
        left: Box<ASTNode>,
        right: Box<ASTNode>,
    },
    UnaryExpr {
        operator: super::lexer::TokenType,
        operand: Box<ASTNode>,
    },
    Literal {
        literal_type: LiteralType,
        value: LiteralValue,
    },
    Identifier {
        name: String,
    },
    CallExpr {
        name: String,
        args: Vec<ASTNode>,
    },
    Assign {
        name: String,
        value: Box<ASTNode>,
    },
    ArrayExpr {
        elements: Vec<ASTNode>,
    },
    IndexExpr {
        array: Box<ASTNode>,
        index: Box<ASTNode>,
    },
    IndexAssign {
        array: Box<ASTNode>,
        index: Box<ASTNode>,
        value: Box<ASTNode>,
    },
    StringConcat {
        left: Box<ASTNode>,
        right: Box<ASTNode>,
    },
    MatchExpr {
        value: Box<ASTNode>,
        patterns: Vec<ASTNode>,
        bodies: Vec<ASTNode>,
    },
    ExternFn {
        name: String,
        lib_name: Option<String>,
        symbol_name: String,
        params: Vec<String>,
        returns_int: i32,
    },
    TryStmt {
        try_block: Box<ASTNode>,
        catch_var: Option<String>,
        catch_block: Option<Box<ASTNode>>,
        finally_block: Option<Box<ASTNode>>,
    },
    TypeDecl {
        name: String,
        fields: Vec<String>,
    },
    FieldAccess {
        object: Box<ASTNode>,
        field: String,
    },
    AddressOf {
        operand: Box<ASTNode>,
    },
    Dereference {
        operand: Box<ASTNode>,
    },
}

#[derive(Debug, Clone)]
pub enum LiteralValue {
    Int(i64),
    Float(f64),
    String(String),
    Bool(bool),
}

pub fn ast_node_type_to_string(node_type: &ASTNodeType) -> &'static str {
    match node_type {
        ASTNodeType::Program => "PROGRAM",
        ASTNodeType::VarDecl => "VAR_DECL",
        ASTNodeType::FnDecl => "FN_DECL",
        ASTNodeType::Block => "BLOCK",
        ASTNodeType::IfStmt => "IF_STMT",
        ASTNodeType::WhileStmt => "WHILE_STMT",
        ASTNodeType::ForStmt => "FOR_STMT",
        ASTNodeType::BreakStmt => "BREAK_STMT",
        ASTNodeType::ContinueStmt => "CONTINUE_STMT",
        ASTNodeType::ReturnStmt => "RETURN_STMT",
        ASTNodeType::PrintStmt => "PRINT_STMT",
        ASTNodeType::ExprStmt => "EXPR_STMT",
        ASTNodeType::BinaryExpr => "BINARY_EXPR",
        ASTNodeType::UnaryExpr => "UNARY_EXPR",
        ASTNodeType::Literal => "LITERAL",
        ASTNodeType::Identifier => "IDENTIFIER",
        ASTNodeType::CallExpr => "CALL_EXPR",
        ASTNodeType::Assign => "ASSIGN",
        ASTNodeType::ArrayExpr => "ARRAY_EXPR",
        ASTNodeType::IndexExpr => "INDEX_EXPR",
        ASTNodeType::IndexAssign => "INDEX_ASSIGN",
        ASTNodeType::StringConcat => "STRING_CONCAT",
        ASTNodeType::MatchExpr => "MATCH_EXPR",
        ASTNodeType::ExternFn => "EXTERN_FN",
        ASTNodeType::TryStmt => "TRY_STMT",
        ASTNodeType::TypeDecl => "TYPE_DECL",
        ASTNodeType::FieldAccess => "FIELD_ACCESS",
        ASTNodeType::AddressOf => "ADDRESS_OF",
        ASTNodeType::Dereference => "DEREFERENCE",
    }
}
