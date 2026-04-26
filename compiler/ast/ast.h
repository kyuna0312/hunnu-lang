#ifndef HUNNU_AST_H
#define HUNNU_AST_H

#include <stddef.h>
#include <stdint.h>
#include "lexer/token.h"

typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_FN_DECL,
    AST_BLOCK,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_PRINT_STMT,
    AST_EXPR_STMT,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_CALL_EXPR,
    AST_ASSIGN,
    AST_ARRAY_EXPR,
    AST_INDEX_EXPR,
    AST_STRING_CONCAT
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    int32_t line;
    int32_t column;
    
    union {
        struct {
            struct ASTNode** statements;
            size_t count;
        } program;
        
        struct {
            char* name;
            struct ASTNode* initializer;
        } var_decl;
        
        struct {
            char* name;
            char** params;
            size_t param_count;
            struct ASTNode* body;
        } fn_decl;
        
        struct {
            struct ASTNode** statements;
            size_t count;
        } block;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;
        
        struct {
            struct ASTNode* initializer;
            struct ASTNode* condition;
            struct ASTNode* update;
            struct ASTNode* body;
        } for_stmt;
        
        struct {
            struct ASTNode* value;
        } return_stmt;
        
        struct {
            struct ASTNode* argument;
        } print_stmt;
        
        struct {
            struct ASTNode* expression;
        } expr_stmt;
        
        struct {
            TokenType operator;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_expr;
        
        struct {
            TokenType operator;
            struct ASTNode* operand;
        } unary_expr;
        
        struct {
            TokenType literal_type;
            union {
                int64_t int_value;
                char* string_value;
                int bool_value;
            } value;
        } literal;
        
        struct {
            char* name;
        } identifier;
        
        struct {
            char* name;
            struct ASTNode** args;
            size_t arg_count;
        } call_expr;
        
        struct {
            char* name;
            struct ASTNode* value;
        } assign;
        
        struct {
            struct ASTNode** elements;
            size_t count;
        } array_expr;
        
        struct {
            struct ASTNode* array;
            struct ASTNode* index;
        } index_expr;
        
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
        } string_concat;
    } data;
} ASTNode;

ASTNode* ast_program_create(ASTNode** statements, size_t count);
ASTNode* ast_var_decl_create(const char* name, ASTNode* initializer, int32_t line, int32_t column);
ASTNode* ast_fn_decl_create(const char* name, char** params, size_t param_count, ASTNode* body, int32_t line, int32_t column);
ASTNode* ast_block_create(ASTNode** statements, size_t count, int32_t line, int32_t column);
ASTNode* ast_if_stmt_create(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, int32_t line, int32_t column);
ASTNode* ast_while_stmt_create(ASTNode* condition, ASTNode* body, int32_t line, int32_t column);
ASTNode* ast_for_stmt_create(ASTNode* initializer, ASTNode* condition, ASTNode* update, ASTNode* body, int32_t line, int32_t column);
ASTNode* ast_return_stmt_create(ASTNode* value, int32_t line, int32_t column);
ASTNode* ast_print_stmt_create(ASTNode* argument, int32_t line, int32_t column);
ASTNode* ast_expr_stmt_create(ASTNode* expression, int32_t line, int32_t column);
ASTNode* ast_binary_expr_create(TokenType operator, ASTNode* left, ASTNode* right, int32_t line, int32_t column);
ASTNode* ast_unary_expr_create(TokenType operator, ASTNode* operand, int32_t line, int32_t column);
ASTNode* ast_literal_create_int(int64_t value, int32_t line, int32_t column);
ASTNode* ast_literal_create_string(char* value, int32_t line, int32_t column);
ASTNode* ast_literal_create_bool(int value, int32_t line, int32_t column);
ASTNode* ast_identifier_create(const char* name, int32_t line, int32_t column);
ASTNode* ast_call_expr_create(const char* name, ASTNode** args, size_t arg_count, int32_t line, int32_t column);
ASTNode* ast_assign_create(const char* name, ASTNode* value, int32_t line, int32_t column);
ASTNode* ast_array_expr_create(ASTNode** elements, size_t count, int32_t line, int32_t column);
ASTNode* ast_index_expr_create(ASTNode* array, ASTNode* index, int32_t line, int32_t column);
ASTNode* ast_string_concat_create(ASTNode* left, ASTNode* right, int32_t line, int32_t column);

void ast_free(ASTNode* node);
void ast_print(ASTNode* node, int indent);
const char* ast_node_type_to_string(ASTNodeType type);

#endif