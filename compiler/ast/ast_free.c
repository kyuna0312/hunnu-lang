#include "ast.h"
#include <stdlib.h>
#include <string.h>

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

        case AST_INDEX_ASSIGN:
            ast_free_node(node->data.index_assign.array);
            ast_free_node(node->data.index_assign.index);
            ast_free_node(node->data.index_assign.value);
            break;

        case AST_STRING_CONCAT:
            ast_free_node(node->data.string_concat.left);
            ast_free_node(node->data.string_concat.right);
            break;

        case AST_MATCH_EXPR:
            ast_free_node(node->data.match_expr.value);
            for (size_t i = 0; i < node->data.match_expr.case_count; i++) {
                ast_free_node(node->data.match_expr.patterns[i]);
                ast_free_node(node->data.match_expr.bodies[i]);
            }
            free(node->data.match_expr.patterns);
            free(node->data.match_expr.bodies);
            break;

        case AST_EXTERN_FN:
            free(node->data.extern_fn.name);
            if (node->data.extern_fn.lib_name) free(node->data.extern_fn.lib_name);
            free(node->data.extern_fn.symbol_name);
            for (size_t i = 0; i < node->data.extern_fn.param_count; i++) {
                free(node->data.extern_fn.param_names[i]);
            }
            if (node->data.extern_fn.param_count > 0) {
                free(node->data.extern_fn.param_names);
            }
            break;

        case AST_TYPE_DECL:
            free(node->data.type_decl.name);
            for (size_t i = 0; i < node->data.type_decl.field_count; i++) {
                free(node->data.type_decl.fields[i]);
            }
            if (node->data.type_decl.field_count > 0) {
                free(node->data.type_decl.fields);
            }
            break;

        case AST_FIELD_ACCESS:
            ast_free_node(node->data.field_access.object);
            free(node->data.field_access.field);
            break;

        case AST_ADDRESS_OF:
            ast_free_node(node->data.address_of.operand);
            break;

        case AST_DEREFERENCE:
            ast_free_node(node->data.dereference.operand);
            break;

        case AST_WHILE_STMT:
        case AST_FOR_STMT:
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
        case AST_RETURN_STMT:
        case AST_TRY_STMT:
            break;

        default:
            break;
    }

    free(node);
}

void ast_free(ASTNode* node) {
    ast_free_node(node);
}
