#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char* ast_type_names[] = {
    "PROGRAM",
    "VAR_DECL",
    "FN_DECL",
    "BLOCK",
    "IF_STMT",
    "WHILE_STMT",
    "FOR_STMT",
    "RETURN_STMT",
    "PRINT_STMT",
    "EXPR_STMT",
    "BINARY_EXPR",
    "UNARY_EXPR",
    "LITERAL",
    "IDENTIFIER",
    "CALL_EXPR",
    "ASSIGN",
    "ARRAY_EXPR",
    "INDEX_EXPR",
    "STRING_CONCAT"
};

const char* ast_node_type_to_string(ASTNodeType type) {
    if (type >= 0 && type <= AST_STRING_CONCAT) {
        return ast_type_names[type];
    }
    return "UNKNOWN";
}

static void indent_print(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

ASTNode* ast_program_create(ASTNode** statements, size_t count) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->line = 1;
    node->column = 1;
    node->data.program.statements = statements;
    node->data.program.count = count;
    return node;
}

ASTNode* ast_var_decl_create(const char* name, ASTNode* initializer, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->line = line;
    node->column = column;
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.initializer = initializer;
    return node;
}

ASTNode* ast_fn_decl_create(const char* name, char** params, size_t param_count, ASTNode* body, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_FN_DECL;
    node->line = line;
    node->column = column;
    node->data.fn_decl.name = strdup(name);
    node->data.fn_decl.params = params;
    node->data.fn_decl.param_count = param_count;
    node->data.fn_decl.body = body;
    return node;
}

ASTNode* ast_block_create(ASTNode** statements, size_t count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;
    node->line = line;
    node->column = column;
    node->data.block.statements = statements;
    node->data.block.count = count;
    return node;
}

ASTNode* ast_if_stmt_create(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_IF_STMT;
    node->line = line;
    node->column = column;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_while_stmt_create(ASTNode* condition, ASTNode* body, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_WHILE_STMT;
    node->line = line;
    node->column = column;
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode* ast_for_stmt_create(ASTNode* initializer, ASTNode* condition, ASTNode* update, ASTNode* body, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_FOR_STMT;
    node->line = line;
    node->column = column;
    node->data.for_stmt.initializer = initializer;
    node->data.for_stmt.condition = condition;
    node->data.for_stmt.update = update;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode* ast_return_stmt_create(ASTNode* value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_RETURN_STMT;
    node->line = line;
    node->column = column;
    node->data.return_stmt.value = value;
    return node;
}

ASTNode* ast_print_stmt_create(ASTNode* argument, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_PRINT_STMT;
    node->line = line;
    node->column = column;
    node->data.print_stmt.argument = argument;
    return node;
}

ASTNode* ast_expr_stmt_create(ASTNode* expression, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_EXPR_STMT;
    node->line = line;
    node->column = column;
    node->data.expr_stmt.expression = expression;
    return node;
}

ASTNode* ast_binary_expr_create(TokenType operator, ASTNode* left, ASTNode* right, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_BINARY_EXPR;
    node->line = line;
    node->column = column;
    node->data.binary_expr.operator = operator;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    return node;
}

ASTNode* ast_unary_expr_create(TokenType operator, ASTNode* operand, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_UNARY_EXPR;
    node->line = line;
    node->column = column;
    node->data.unary_expr.operator = operator;
    node->data.unary_expr.operand = operand;
    return node;
}

ASTNode* ast_literal_create_int(int64_t value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_LITERAL;
    node->line = line;
    node->column = column;
    node->data.literal.literal_type = TOKEN_INT_LITERAL;
    node->data.literal.value.int_value = value;
    return node;
}

ASTNode* ast_literal_create_string(char* value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_LITERAL;
    node->line = line;
    node->column = column;
    node->data.literal.literal_type = TOKEN_STRING_LITERAL;
    node->data.literal.value.string_value = strdup(value);
    return node;
}

ASTNode* ast_literal_create_bool(int value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_LITERAL;
    node->line = line;
    node->column = column;
    node->data.literal.literal_type = TOKEN_BOOL_LITERAL;
    node->data.literal.value.bool_value = value;
    return node;
}

ASTNode* ast_identifier_create(const char* name, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_IDENTIFIER;
    node->line = line;
    node->column = column;
    node->data.identifier.name = strdup(name);
    return node;
}

ASTNode* ast_call_expr_create(const char* name, ASTNode** args, size_t arg_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_CALL_EXPR;
    node->line = line;
    node->column = column;
    node->data.call_expr.name = strdup(name);
    node->data.call_expr.args = args;
    node->data.call_expr.arg_count = arg_count;
    return node;
}

ASTNode* ast_assign_create(const char* name, ASTNode* value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_ASSIGN;
    node->line = line;
    node->column = column;
    node->data.assign.name = strdup(name);
    node->data.assign.value = value;
    return node;
}

ASTNode* ast_array_expr_create(ASTNode** elements, size_t count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_ARRAY_EXPR;
    node->line = line;
    node->column = column;
    node->data.array_expr.elements = elements;
    node->data.array_expr.count = count;
    return node;
}

ASTNode* ast_index_expr_create(ASTNode* array, ASTNode* index, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_INDEX_EXPR;
    node->line = line;
    node->column = column;
    node->data.index_expr.array = array;
    node->data.index_expr.index = index;
    return node;
}

ASTNode* ast_string_concat_create(ASTNode* left, ASTNode* right, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_STRING_CONCAT;
    node->line = line;
    node->column = column;
    node->data.string_concat.left = left;
    node->data.string_concat.right = right;
    return node;
}

static void ast_free_node(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < node->data.program.count; i++) {
                ast_free_node(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;
            
        case AST_VAR_DECL:
            free(node->data.var_decl.name);
            ast_free_node(node->data.var_decl.initializer);
            break;
            
        case AST_FN_DECL:
            free(node->data.fn_decl.name);
            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                free(node->data.fn_decl.params[i]);
            }
            free(node->data.fn_decl.params);
            ast_free_node(node->data.fn_decl.body);
            break;
            
        case AST_BLOCK:
            for (size_t i = 0; i < node->data.block.count; i++) {
                ast_free_node(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
            
        case AST_IF_STMT:
            ast_free_node(node->data.if_stmt.condition);
            ast_free_node(node->data.if_stmt.then_branch);
            ast_free_node(node->data.if_stmt.else_branch);
            break;
            
        case AST_PRINT_STMT:
            ast_free_node(node->data.print_stmt.argument);
            break;
            
        case AST_EXPR_STMT:
            ast_free_node(node->data.expr_stmt.expression);
            break;
            
        case AST_BINARY_EXPR:
            ast_free_node(node->data.binary_expr.left);
            ast_free_node(node->data.binary_expr.right);
            break;
            
        case AST_UNARY_EXPR:
            ast_free_node(node->data.unary_expr.operand);
            break;
            
        case AST_LITERAL:
            if (node->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                free(node->data.literal.value.string_value);
            }
            break;
            
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_CALL_EXPR:
            free(node->data.call_expr.name);
            for (size_t i = 0; i < node->data.call_expr.arg_count; i++) {
                ast_free_node(node->data.call_expr.args[i]);
            }
            free(node->data.call_expr.args);
            break;
            
        case AST_ASSIGN:
            free(node->data.assign.name);
            ast_free_node(node->data.assign.value);
            break;
            
        case AST_ARRAY_EXPR:
            for (size_t i = 0; i < node->data.array_expr.count; i++) {
                ast_free_node(node->data.array_expr.elements[i]);
            }
            free(node->data.array_expr.elements);
            break;
            
        case AST_INDEX_EXPR:
            ast_free_node(node->data.index_expr.array);
            ast_free_node(node->data.index_expr.index);
            break;
            
        case AST_STRING_CONCAT:
            ast_free_node(node->data.string_concat.left);
            ast_free_node(node->data.string_concat.right);
            break;
    }
    
    free(node);
}

void ast_free(ASTNode* node) {
    ast_free_node(node);
}

static void ast_print_node(ASTNode* node, int indent) {
    if (!node) {
        indent_print(indent);
        printf("NULL\n");
        return;
    }
    
    indent_print(indent);
    printf("%s", ast_node_type_to_string(node->type));
    
    switch (node->type) {
        case AST_VAR_DECL:
            printf(" (%s)", node->data.var_decl.name);
            break;
            
        case AST_FN_DECL:
            printf(" (%s)", node->data.fn_decl.name);
            break;
            
        case AST_IDENTIFIER:
            printf(" (%s)", node->data.identifier.name);
            break;
            
        case AST_LITERAL:
            if (node->data.literal.literal_type == TOKEN_INT_LITERAL) {
                printf(" (%ld)", (long)node->data.literal.value.int_value);
            } else if (node->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                printf(" (\"%s\")", node->data.literal.value.string_value);
            } else if (node->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                printf(" (%s)", node->data.literal.value.bool_value ? "true" : "false");
            }
            break;
            
        case AST_BINARY_EXPR:
            printf(" (%s)", token_type_to_string(node->data.binary_expr.operator));
            break;
            
        case AST_CALL_EXPR:
            printf(" (%s)", node->data.call_expr.name);
            break;
            
        case AST_ASSIGN:
            printf(" (%s)", node->data.assign.name);
            break;
    }
    
    printf("\n");
    
    switch (node->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < node->data.program.count; i++) {
                ast_print_node(node->data.program.statements[i], indent + 1);
            }
            break;
            
        case AST_VAR_DECL:
            ast_print_node(node->data.var_decl.initializer, indent + 1);
            break;
            
        case AST_FN_DECL:
            ast_print_node(node->data.fn_decl.body, indent + 1);
            break;
            
        case AST_BLOCK:
            for (size_t i = 0; i < node->data.block.count; i++) {
                ast_print_node(node->data.block.statements[i], indent + 1);
            }
            break;
            
        case AST_IF_STMT:
            indent_print(indent + 1);
            printf("condition:\n");
            ast_print_node(node->data.if_stmt.condition, indent + 2);
            indent_print(indent + 1);
            printf("then:\n");
            ast_print_node(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                indent_print(indent + 1);
                printf("else:\n");
                ast_print_node(node->data.if_stmt.else_branch, indent + 2);
            }
            break;
            
        case AST_PRINT_STMT:
            ast_print_node(node->data.print_stmt.argument, indent + 1);
            break;
            
        case AST_EXPR_STMT:
            ast_print_node(node->data.expr_stmt.expression, indent + 1);
            break;
            
        case AST_BINARY_EXPR:
            ast_print_node(node->data.binary_expr.left, indent + 1);
            ast_print_node(node->data.binary_expr.right, indent + 1);
            break;
            
        case AST_UNARY_EXPR:
            ast_print_node(node->data.unary_expr.operand, indent + 1);
            break;
            
        case AST_CALL_EXPR:
            for (size_t i = 0; i < node->data.call_expr.arg_count; i++) {
                ast_print_node(node->data.call_expr.args[i], indent + 1);
            }
            break;
            
        case AST_ASSIGN:
            ast_print_node(node->data.assign.value, indent + 1);
            break;
            
        case AST_ARRAY_EXPR:
            for (size_t i = 0; i < node->data.array_expr.count; i++) {
                ast_print_node(node->data.array_expr.elements[i], indent + 1);
            }
            break;
            
        case AST_INDEX_EXPR:
            ast_print_node(node->data.index_expr.array, indent + 1);
            ast_print_node(node->data.index_expr.index, indent + 1);
            break;
            
        case AST_STRING_CONCAT:
            ast_print_node(node->data.string_concat.left, indent + 1);
            ast_print_node(node->data.string_concat.right, indent + 1);
            break;
            
        default:
            break;
    }
}

void ast_print(ASTNode* node, int indent) {
    ast_print_node(node, indent);
}