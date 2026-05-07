/**
 * @file ast.h
 * @brief Abstract Syntax Tree node definitions for Hunnu
 */

#ifndef HUNNU_AST_H
#define HUNNU_AST_H

#include <stddef.h>
#include <stdint.h>
#include "lexer/token.h"

/** AST node types */
typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_FN_DECL,
    AST_BLOCK,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_BREAK_STMT,
    AST_CONTINUE_STMT,
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
    AST_INDEX_ASSIGN,
    AST_STRING_CONCAT,
    AST_MATCH_EXPR,
    AST_EXTERN_FN,
    AST_TRY_STMT,
    AST_TYPE_DECL,
    AST_FIELD_ACCESS,
    AST_ADDRESS_OF,
    AST_DEREFERENCE,
} ASTNodeType;

/** AST node structure */
typedef struct ASTNode {
    ASTNodeType type;    /**< Node type */
    int32_t line;        /**< Line number */
    int32_t column;     /**< Column number */
    
    union {
        /** Program node - list of top-level statements */
        struct {
            struct ASTNode** statements;
            size_t count;
        } program;
        
        /** Variable declaration */
        struct {
            char* name;
            struct ASTNode* initializer;
        } var_decl;
        
        /** Function declaration */
        struct {
            char* name;
            char** params;
            size_t param_count;
            struct ASTNode* body;
        } fn_decl;
        
        /** Block statement */
        struct {
            struct ASTNode** statements;
            size_t count;
        } block;
        
        /** If statement */
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;
        
        /** While loop */
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;
        
        /** For loop */
        struct {
            struct ASTNode* initializer;
            struct ASTNode* condition;
            struct ASTNode* update;
            struct ASTNode* body;
        } for_stmt;
        
        /** Return statement */
        struct {
            struct ASTNode* value;
        } return_stmt;
        
        /** Print statement */
        struct {
            struct ASTNode* argument;
        } print_stmt;
        
        /** Expression statement */
        struct {
            struct ASTNode* expression;
        } expr_stmt;
        
        /** Binary expression */
        struct {
            TokenType operator;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_expr;
        
        /** Unary expression */
        struct {
            TokenType operator;
            struct ASTNode* operand;
        } unary_expr;
        
        /** Literal value */
        struct {
            TokenType literal_type;
            union {
                int64_t int_value;
                double float_value;
                char* string_value;
                int bool_value;
            } value;
        } literal;
        
        /** Identifier reference */
        struct {
            char* name;
        } identifier;
        
        /** Function call */
        struct {
            char* name;
            struct ASTNode** args;
            size_t arg_count;
        } call_expr;
        
        /** Assignment */
        struct {
            char* name;
            struct ASTNode* value;
        } assign;
        
        /** Array literal */
        struct {
            struct ASTNode** elements;
            size_t count;
        } array_expr;
        
        /** Array index access */
        struct {
            struct ASTNode* array;
            struct ASTNode* index;
        } index_expr;
        
        /** Array index assignment */
        struct {
            struct ASTNode* array;
            struct ASTNode* index;
            struct ASTNode* value;
        } index_assign;
        
        /** String concatenation */
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
        } string_concat;
        
        /** Match expression */
        struct {
            struct ASTNode* value;
            struct ASTNode** patterns;
            struct ASTNode** bodies;
            size_t case_count;
        } match_expr;
        
        /** External function declaration */
        struct {
            char* name;           /* Hunnu function name */
            char* lib_name;       /* Library name (e.g., "libc.so.6" or NULL) */
            char* symbol_name;    /* C symbol name (e.g., "puts") */
            char** param_names;   /* Parameter names */
            size_t param_count;
            int returns_int;      /* 1 if returns int, 0 if void */
        } extern_fn;

        /** Try-catch statement */
        struct {
            struct ASTNode* try_block;      /* Try block */
            char* catch_var;               /* Catch variable name (or NULL) */
            struct ASTNode* catch_block;     /* Catch block */
            struct ASTNode* finally_block;   /* Finally block (or NULL) */
        } try_stmt;

        /** Type/struct declaration */
        struct {
            char* name;                    /* Struct name */
            char** fields;                 /* Field names */
            size_t field_count;            /* Number of fields */
        } type_decl;

        /** Field access: obj.field */
        struct {
            struct ASTNode* object;         /* Object expression */
            char* field;                   /* Field name */
        } field_access;

        /** Address-of: &expr */
        struct {
            struct ASTNode* operand;
        } address_of;

        /** Dereference: *expr */
        struct {
            struct ASTNode* operand;
        } dereference;
    } data;
} ASTNode;

/* AST creation functions */
ASTNode* ast_program_create(ASTNode** statements, size_t count);
ASTNode* ast_var_decl_create(const char* name, ASTNode* initializer, int32_t line, int32_t column);
ASTNode* ast_fn_decl_create(const char* name, char** params, size_t param_count, ASTNode* body, int32_t line, int32_t column);
ASTNode* ast_block_create(ASTNode** statements, size_t count, int32_t line, int32_t column);
ASTNode* ast_if_stmt_create(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, int32_t line, int32_t column);
ASTNode* ast_while_stmt_create(ASTNode* condition, ASTNode* body, int32_t line, int32_t column);
ASTNode* ast_for_stmt_create(ASTNode* initializer, ASTNode* condition, ASTNode* update, ASTNode* body, int32_t line, int32_t column);
ASTNode* ast_break_stmt_create(int32_t line, int32_t column);
ASTNode* ast_continue_stmt_create(int32_t line, int32_t column);
ASTNode* ast_return_stmt_create(ASTNode* value, int32_t line, int32_t column);
ASTNode* ast_print_stmt_create(ASTNode* argument, int32_t line, int32_t column);
ASTNode* ast_expr_stmt_create(ASTNode* expression, int32_t line, int32_t column);
ASTNode* ast_binary_expr_create(TokenType operator, ASTNode* left, ASTNode* right, int32_t line, int32_t column);
ASTNode* ast_unary_expr_create(TokenType operator, ASTNode* operand, int32_t line, int32_t column);
ASTNode* ast_literal_create_int(int64_t value, int32_t line, int32_t column);
ASTNode* ast_literal_create_float(double value, int32_t line, int32_t column);
ASTNode* ast_literal_create_string(char* value, int32_t line, int32_t column);
ASTNode* ast_literal_create_bool(int value, int32_t line, int32_t column);
ASTNode* ast_identifier_create(const char* name, int32_t line, int32_t column);
ASTNode* ast_call_expr_create(const char* name, ASTNode** args, size_t arg_count, int32_t line, int32_t column);
ASTNode* ast_assign_create(const char* name, ASTNode* value, int32_t line, int32_t column);
ASTNode* ast_array_expr_create(ASTNode** elements, size_t count, int32_t line, int32_t column);
ASTNode* ast_index_expr_create(ASTNode* array, ASTNode* index, int32_t line, int32_t column);
ASTNode* ast_index_assign_create(ASTNode* array, ASTNode* index, ASTNode* value, int32_t line, int32_t column);
ASTNode* ast_string_concat_create(ASTNode* left, ASTNode* right, int32_t line, int32_t column);
ASTNode* ast_match_expr_create(ASTNode* value, ASTNode** patterns, ASTNode** bodies,
                               size_t case_count, int32_t line, int32_t column);
ASTNode* ast_extern_fn_create(const char* name, const char* lib_name, const char* symbol_name,
                               char** param_names, size_t param_count, int returns_int,
                               int32_t line, int32_t column);
ASTNode* ast_try_stmt_create(ASTNode* try_block, const char* catch_var,
                               ASTNode* catch_block, ASTNode* finally_block,
                               int32_t line, int32_t column);
ASTNode* ast_type_decl_create(const char* name, char** fields, size_t field_count,
                              int32_t line, int32_t column);
ASTNode* ast_field_access_create(ASTNode* object, const char* field,
                                  int32_t line, int32_t column);
ASTNode* ast_address_of_create(ASTNode* operand, int32_t line, int32_t column);
ASTNode* ast_dereference_create(ASTNode* operand, int32_t line, int32_t column);

void ast_free(ASTNode* node);
void ast_print(ASTNode* node, int indent);
const char* ast_node_type_to_string(ASTNodeType type);

#endif