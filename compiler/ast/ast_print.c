#include "ast.h"
#include <stdio.h>

static void indent_print(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
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
            printf(" (op)");
            break;

        case AST_CALL_EXPR:
            printf(" (%s)", node->data.call_expr.name);
            break;

        case AST_ASSIGN:
            printf(" (%s)", node->data.assign.name);
            break;

        case AST_EXTERN_FN:
            printf(" (%s -> %s", node->data.extern_fn.name, node->data.extern_fn.symbol_name);
            if (node->data.extern_fn.lib_name) {
                printf(" from %s", node->data.extern_fn.lib_name);
            }
            printf(")");
            break;

        case AST_MATCH_EXPR:
            printf(" (value)");
            break;

        default:
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

        case AST_INDEX_ASSIGN:
            ast_print_node(node->data.index_assign.array, indent + 1);
            ast_print_node(node->data.index_assign.index, indent + 1);
            ast_print_node(node->data.index_assign.value, indent + 1);
            break;

        case AST_STRING_CONCAT:
            ast_print_node(node->data.string_concat.left, indent + 1);
            ast_print_node(node->data.string_concat.right, indent + 1);
            break;

        case AST_MATCH_EXPR:
            ast_print_node(node->data.match_expr.value, indent + 1);
            printf(" match cases:\n");
            for (size_t i = 0; i < node->data.match_expr.case_count; i++) {
                indent_print(indent + 1);
                printf("pattern: ");
                ast_print_node(node->data.match_expr.patterns[i], 0);
                printf("\n");
                indent_print(indent + 1);
                printf("body: ");
                ast_print_node(node->data.match_expr.bodies[i], 0);
                printf("\n");
            }
            break;

        case AST_EXTERN_FN:
            break;

        case AST_TRY_STMT:
            indent_print(indent);
            printf("TRY_STMT:\n");
            indent_print(indent + 1);
            printf("try_block:\n");
            ast_print_node(node->data.try_stmt.try_block, indent + 2);
            printf("\n");
            if (node->data.try_stmt.catch_var) {
                indent_print(indent + 1);
                printf("catch_var: %s\n", node->data.try_stmt.catch_var);
            }
            if (node->data.try_stmt.catch_block) {
                indent_print(indent + 1);
                printf("catch_block:\n");
                ast_print_node(node->data.try_stmt.catch_block, indent + 2);
                printf("\n");
            }
            if (node->data.try_stmt.finally_block) {
                indent_print(indent + 1);
                printf("finally_block:\n");
                ast_print_node(node->data.try_stmt.finally_block, indent + 2);
                printf("\n");
            }
            break;

        case AST_TYPE_DECL:
            indent_print(indent);
            printf("TYPE_DECL (%s) with %zu fields\n", node->data.type_decl.name, node->data.type_decl.field_count);
            for (size_t i = 0; i < node->data.type_decl.field_count; i++) {
                indent_print(indent + 1);
                printf("field: %s\n", node->data.type_decl.fields[i]);
            }
            break;

        case AST_FIELD_ACCESS:
            indent_print(indent);
            printf("FIELD_ACCESS (.%s)\n", node->data.field_access.field);
            ast_print_node(node->data.field_access.object, indent + 1);
            break;

        case AST_ADDRESS_OF:
            indent_print(indent);
            printf("ADDRESS_OF (&)\n");
            ast_print_node(node->data.address_of.operand, indent + 1);
            break;

        case AST_DEREFERENCE:
            indent_print(indent);
            printf("DEREFERENCE (*)\n");
            ast_print_node(node->data.dereference.operand, indent + 1);
            break;

        default:
            break;
    }
}

void ast_print(ASTNode* node, int indent) {
    ast_print_node(node, indent);
}
