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
            for (size_t i = 0; i < node->data.fn_decl.type_param_count; i++) {
                free(node->data.fn_decl.type_params[i]);
            }
            free(node->data.fn_decl.type_params);
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
            if (node->data.literal.literal_type == TOKEN_STRING_LITERAL ||
                node->data.literal.literal_type == TOKEN_SYMBOL) {
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
                free(node->data.type_decl.is_pub);
            }
            for (size_t i = 0; i < node->data.type_decl.type_param_count; i++) {
                free(node->data.type_decl.type_params[i]);
            }
            free(node->data.type_decl.type_params);
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

        case AST_STRUCT_INSTANCE:
            free(node->data.struct_instance.type_name);
            for (size_t i = 0; i < node->data.struct_instance.field_count; i++) {
                free(node->data.struct_instance.field_names[i]);
                ast_free_node(node->data.struct_instance.field_values[i]);
            }
            if (node->data.struct_instance.field_count > 0) {
                free(node->data.struct_instance.field_names);
                free(node->data.struct_instance.field_values);
            }
            break;

        case AST_METHOD_CALL:
            ast_free_node(node->data.method_call.object);
            free(node->data.method_call.method);
            for (size_t i = 0; i < node->data.method_call.arg_count; i++) {
                ast_free_node(node->data.method_call.args[i]);
            }
            if (node->data.method_call.arg_count > 0) {
                free(node->data.method_call.args);
            }
            break;

        case AST_CLASS_DECL:
            free(node->data.class_decl.name);
            free(node->data.class_decl.parent_name);
            for (size_t i = 0; i < node->data.class_decl.field_count; i++) {
                free(node->data.class_decl.fields[i]);
            }
            if (node->data.class_decl.field_count > 0) {
                free(node->data.class_decl.fields);
                free(node->data.class_decl.is_pub);
            }
            for (size_t i = 0; i < node->data.class_decl.type_param_count; i++) {
                free(node->data.class_decl.type_params[i]);
            }
            free(node->data.class_decl.type_params);
            ast_free_node(node->data.class_decl.constructor);
            for (size_t i = 0; i < node->data.class_decl.method_count; i++) {
                ast_free_node(node->data.class_decl.methods[i]);
            }
            free(node->data.class_decl.methods);
            break;

        case AST_NEW_EXPR:
            free(node->data.new_expr.class_name);
            for (size_t i = 0; i < node->data.new_expr.arg_count; i++) {
                ast_free_node(node->data.new_expr.args[i]);
            }
            if (node->data.new_expr.arg_count > 0) {
                free(node->data.new_expr.args);
            }
            break;

        case AST_FIELD_ASSIGN:
            ast_free_node(node->data.field_assign.object);
            free(node->data.field_assign.field);
            ast_free_node(node->data.field_assign.value);
            break;

        case AST_TRAIT_DECL:
            free(node->data.trait_decl.name);
            for (size_t i = 0; i < node->data.trait_decl.method_count; i++) {
                free(node->data.trait_decl.method_names[i]);
            }
            free(node->data.trait_decl.method_names);
            free(node->data.trait_decl.method_param_counts);
            break;

        case AST_IMPL_DECL:
            free(node->data.impl_decl.trait_name);
            free(node->data.impl_decl.type_name);
            for (size_t i = 0; i < node->data.impl_decl.method_count; i++) {
                ast_free_node(node->data.impl_decl.methods[i]);
            }
            free(node->data.impl_decl.methods);
            break;

        case AST_UNSAFE_BLOCK:
            ast_free_node(node->data.unsafe_block.body);
            break;

        case AST_ENUM_DECL:
            free(node->data.enum_decl.name);
            for (size_t i = 0; i < node->data.enum_decl.type_param_count; i++) {
                free(node->data.enum_decl.type_params[i]);
            }
            free(node->data.enum_decl.type_params);
            for (size_t i = 0; i < node->data.enum_decl.variant_count; i++) {
                free(node->data.enum_decl.variant_names[i]);
                if (node->data.enum_decl.variant_field_names && node->data.enum_decl.variant_field_names[i]) {
                    for (size_t j = 0; j < node->data.enum_decl.variant_field_counts[i]; j++) {
                        free(node->data.enum_decl.variant_field_names[i][j]);
                    }
                    free(node->data.enum_decl.variant_field_names[i]);
                }
            }
            free(node->data.enum_decl.variant_names);
            free(node->data.enum_decl.variant_field_counts);
            free(node->data.enum_decl.variant_field_names);
            break;

        case AST_ENUM_VARIANT:
            free(node->data.enum_variant.enum_name);
            free(node->data.enum_variant.variant_name);
            for (size_t i = 0; i < node->data.enum_variant.arg_count; i++) {
                ast_free_node(node->data.enum_variant.args[i]);
            }
            free(node->data.enum_variant.args);
            break;

        case AST_LAMBDA:
            for (size_t i = 0; i < node->data.lambda.param_count; i++) {
                free(node->data.lambda.params[i]);
            }
            free(node->data.lambda.params);
            ast_free_node(node->data.lambda.body);
            break;

        case AST_RANGE_PATTERN:
            ast_free_node(node->data.range_pattern.start);
            ast_free_node(node->data.range_pattern.end);
            break;

        case AST_ARRAY_PATTERN:
            for (size_t i = 0; i < node->data.array_pattern.count; i++) {
                free(node->data.array_pattern.names[i]);
            }
            free(node->data.array_pattern.names);
            free(node->data.array_pattern.rest_name);
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
