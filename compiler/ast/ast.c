/**
 * @file ast.c
 * @brief AST node creation and operations
 */

#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** AST node type names for debugging */
static const char* ast_type_names[] = {
    "PROGRAM",
    "VAR_DECL",
    "FN_DECL",
    "BLOCK",
    "IF_STMT",
    "WHILE_STMT",
    "FOR_STMT",
    "BREAK_STMT",
    "CONTINUE_STMT",
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
    "INDEX_ASSIGN",
    "STRING_CONCAT",
    "MATCH_EXPR",
    "EXTERN_FN",
    "TRY_STMT",
    "TYPE_DECL",
    "FIELD_ACCESS",
    "ADDRESS_OF",
    "DEREFERENCE",
    "STRUCT_INSTANCE",
    "METHOD_CALL",
    "CLASS_DECL",
    "NEW_EXPR",
    "FIELD_ASSIGN",
    "TRAIT_DECL",
    "IMPL_DECL",
    "UNSAFE_BLOCK",
    "ENUM_DECL",
    "ENUM_VARIANT",
    "LAMBDA"
};

/**
 * @brief Converts AST node type to string
 * @param type AST node type
 * @return String name
 */
const char* ast_node_type_to_string(ASTNodeType type) {
    if (type >= 0 && type < (int)(sizeof(ast_type_names) / sizeof(ast_type_names[0]))) {
        return ast_type_names[type];
    }
    return "UNKNOWN";
}

/**
 * @brief Creates a program node
 * @param statements Array of statements
 * @param count Number of statements
 * @return New AST node
 */
ASTNode* ast_program_create(ASTNode** statements, size_t count) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->line = 1;
    node->column = 1;
    node->data.program.statements = statements;
    node->data.program.count = count;
    return node;
}

/**
 * @brief Creates a variable declaration node
 * @param name Variable name
 * @param initializer Initializer expression
 * @param line Line number
 * @param column Column number
 * @return New AST node
 */
ASTNode* ast_var_decl_create(const char* name, ASTNode* initializer, int32_t line, int32_t column, int is_mut) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->line = line;
    node->column = column;
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.initializer = initializer;
    node->data.var_decl.is_mut = is_mut;
    return node;
}

/**
 * @brief Creates a function declaration node
 * @param name Function name
 * @param params Parameter names
 * @param param_count Number of parameters
 * @param body Function body
 * @param line Line number
 * @param column Column number
 * @return New AST node
 */
ASTNode* ast_fn_decl_create(const char* name, char** params, size_t param_count, ASTNode* body, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_FN_DECL;
    node->line = line;
    node->column = column;
    node->data.fn_decl.name = strdup(name);
    node->data.fn_decl.params = params;
    node->data.fn_decl.param_count = param_count;
    node->data.fn_decl.body = body;
    node->data.fn_decl.type_params = NULL;
    node->data.fn_decl.type_param_count = 0;
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

ASTNode* ast_break_stmt_create(int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_BREAK_STMT;
    node->line = line;
    node->column = column;
    return node;
}

ASTNode* ast_continue_stmt_create(int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_CONTINUE_STMT;
    node->line = line;
    node->column = column;
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

ASTNode* ast_literal_create_float(double value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_LITERAL;
    node->line = line;
    node->column = column;
    node->data.literal.literal_type = TOKEN_FLOAT_LITERAL;
    node->data.literal.value.float_value = value;
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

ASTNode* ast_index_assign_create(ASTNode* array, ASTNode* index, ASTNode* value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_INDEX_ASSIGN;
    node->line = line;
    node->column = column;
    node->data.index_assign.array = array;
    node->data.index_assign.index = index;
    node->data.index_assign.value = value;
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

ASTNode* ast_match_expr_create(ASTNode* value, ASTNode** patterns, ASTNode** bodies,
                               size_t case_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_MATCH_EXPR;
    node->line = line;
    node->column = column;
    node->data.match_expr.value = value;
    node->data.match_expr.patterns = patterns;
    node->data.match_expr.bodies = bodies;
    node->data.match_expr.case_count = case_count;
    return node;
}

ASTNode* ast_extern_fn_create(const char* name, const char* lib_name, const char* symbol_name,
                               char** param_names, size_t param_count, int returns_int,
                               int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_EXTERN_FN;
    node->line = line;
    node->column = column;
    node->data.extern_fn.name = strdup(name);
    node->data.extern_fn.lib_name = lib_name ? strdup(lib_name) : NULL;
    node->data.extern_fn.symbol_name = strdup(symbol_name);
    node->data.extern_fn.param_names = param_names;
    node->data.extern_fn.param_count = param_count;
    node->data.extern_fn.returns_int = returns_int;
    return node;
}

ASTNode* ast_try_stmt_create(ASTNode* try_block, const char* catch_var,
                              ASTNode* catch_block, ASTNode* finally_block,
                              int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_TRY_STMT;
    node->line = line;
    node->column = column;
    node->data.try_stmt.try_block = try_block;
    node->data.try_stmt.catch_var = catch_var ? strdup(catch_var) : NULL;
    node->data.try_stmt.catch_block = catch_block;
    node->data.try_stmt.finally_block = finally_block;
    return node;
}

ASTNode* ast_type_decl_create(const char* name, char** fields, int* is_pub, size_t field_count,
                                int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_TYPE_DECL;
    node->line = line;
    node->column = column;
    node->data.type_decl.name = strdup(name);
    node->data.type_decl.fields = fields;
    node->data.type_decl.is_pub = is_pub;
    node->data.type_decl.field_count = field_count;
    node->data.type_decl.type_params = NULL;
    node->data.type_decl.type_param_count = 0;
    return node;
}

ASTNode* ast_field_access_create(ASTNode* object, const char* field,
                                  int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_FIELD_ACCESS;
    node->line = line;
    node->column = column;
    node->data.field_access.object = object;
    node->data.field_access.field = strdup(field);
    return node;
}

ASTNode* ast_address_of_create(ASTNode* operand, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_ADDRESS_OF;
    node->line = line;
    node->column = column;
    node->data.address_of.operand = operand;
    return node;
}

ASTNode* ast_dereference_create(ASTNode* operand, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_DEREFERENCE;
    node->line = line;
    node->column = column;
    node->data.dereference.operand = operand;
    return node;
}

ASTNode* ast_struct_instance_create(const char* type_name, char** field_names,
                                     ASTNode** field_values, size_t field_count,
                                     int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_STRUCT_INSTANCE;
    node->line = line;
    node->column = column;
    node->data.struct_instance.type_name = strdup(type_name);
    node->data.struct_instance.field_names = field_names;
    node->data.struct_instance.field_values = field_values;
    node->data.struct_instance.field_count = field_count;
    return node;
}

ASTNode* ast_method_call_create(ASTNode* object, const char* method,
                                 ASTNode** args, size_t arg_count,
                                 int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_METHOD_CALL;
    node->line = line;
    node->column = column;
    node->data.method_call.object = object;
    node->data.method_call.method = strdup(method);
    node->data.method_call.args = args;
    node->data.method_call.arg_count = arg_count;
    return node;
}

ASTNode* ast_class_decl_create(const char* name, const char* parent_name, char** fields, int* is_pub,
                                 size_t field_count, ASTNode* constructor,
                                 ASTNode** methods, size_t method_count,
                                 int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_CLASS_DECL;
    node->line = line;
    node->column = column;
    node->data.class_decl.name = strdup(name);
    node->data.class_decl.parent_name = parent_name ? strdup(parent_name) : NULL;
    node->data.class_decl.fields = fields;
    node->data.class_decl.is_pub = is_pub;
    node->data.class_decl.field_count = field_count;
    node->data.class_decl.constructor = constructor;
    node->data.class_decl.methods = methods;
    node->data.class_decl.method_count = method_count;
    node->data.class_decl.type_params = NULL;
    node->data.class_decl.type_param_count = 0;
    return node;
}

ASTNode* ast_new_expr_create(const char* class_name, ASTNode** args,
                              size_t arg_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_NEW_EXPR;
    node->line = line;
    node->column = column;
    node->data.new_expr.class_name = strdup(class_name);
    node->data.new_expr.args = args;
    node->data.new_expr.arg_count = arg_count;
    return node;
}

ASTNode* ast_field_assign_create(ASTNode* object, const char* field,
                                   ASTNode* value, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_FIELD_ASSIGN;
    node->line = line;
    node->column = column;
    node->data.field_assign.object = object;
    node->data.field_assign.field = strdup(field);
    node->data.field_assign.value = value;
    return node;
}

ASTNode* ast_trait_decl_create(const char* name, char** method_names, size_t* method_param_counts, size_t method_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_TRAIT_DECL;
    node->line = line;
    node->column = column;
    node->data.trait_decl.name = strdup(name);
    node->data.trait_decl.method_names = method_names;
    node->data.trait_decl.method_param_counts = method_param_counts;
    node->data.trait_decl.method_count = method_count;
    return node;
}

ASTNode* ast_impl_decl_create(const char* trait_name, const char* type_name, ASTNode** methods, size_t method_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_IMPL_DECL;
    node->line = line;
    node->column = column;
    node->data.impl_decl.trait_name = strdup(trait_name);
    node->data.impl_decl.type_name = strdup(type_name);
    node->data.impl_decl.methods = methods;
    node->data.impl_decl.method_count = method_count;
    return node;
}

ASTNode* ast_unsafe_block_create(ASTNode* body, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_UNSAFE_BLOCK;
    node->line = line;
    node->column = column;
    node->data.unsafe_block.body = body;
    return node;
}

ASTNode* ast_enum_decl_create(const char* name, char** variant_names, size_t* variant_field_counts, char*** variant_field_names, size_t variant_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_ENUM_DECL;
    node->line = line;
    node->column = column;
    node->data.enum_decl.name = strdup(name);
    node->data.enum_decl.variant_names = variant_names;
    node->data.enum_decl.variant_field_counts = variant_field_counts;
    node->data.enum_decl.variant_field_names = variant_field_names;
    node->data.enum_decl.variant_count = variant_count;
    node->data.enum_decl.type_params = NULL;
    node->data.enum_decl.type_param_count = 0;
    return node;
}

ASTNode* ast_enum_variant_create(const char* enum_name, const char* variant_name, ASTNode** args, size_t arg_count, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_ENUM_VARIANT;
    node->line = line;
    node->column = column;
    node->data.enum_variant.enum_name = strdup(enum_name);
    node->data.enum_variant.variant_name = strdup(variant_name);
    node->data.enum_variant.args = args;
    node->data.enum_variant.arg_count = arg_count;
    return node;
}

ASTNode* ast_lambda_create(char** params, size_t param_count, ASTNode* body, int32_t line, int32_t column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_LAMBDA;
    node->line = line;
    node->column = column;
    node->data.lambda.params = params;
    node->data.lambda.param_count = param_count;
    node->data.lambda.body = body;
    return node;
}

