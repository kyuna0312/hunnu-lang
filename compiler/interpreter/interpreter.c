/**
 * @file interpreter.c
 * @brief AST interpreter implementation
 */

#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** Variable scope */
typedef struct Scope {
    char** names;
    Value* values;
    size_t capacity;
    size_t count;
    struct Scope* parent;
} Scope;

/** Interpreter state */
struct Interpreter {
    Scope* current_scope;
    Value return_value;
    int has_return;
    int has_break;
    int has_continue;
};

/**
 * @brief Creates a new scope
 * @param capacity Initial capacity
 * @param parent Parent scope
 * @return New scope
 */
static Scope* scope_create(size_t capacity, Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->capacity = capacity;
    scope->count = 0;
    scope->parent = parent;
    scope->names = (char**)malloc(sizeof(char*) * capacity);
    scope->values = (Value*)malloc(sizeof(Value) * capacity);
    return scope;
}

static void scope_free(Scope* scope) {
    if (!scope) return;
    for (size_t i = 0; i < scope->count; i++) {
        free(scope->names[i]);
        value_free(&scope->values[i]);
    }
    free(scope->names);
    free(scope->values);
    free(scope);
}

static Value* scope_lookup(Scope* scope, const char* name) {
    Scope* current = scope;
    while (current) {
        for (size_t i = 0; i < current->count; i++) {
            if (strcmp(current->names[i], name) == 0) {
                return &current->values[i];
            }
        }
        current = current->parent;
    }
    return NULL;
}

static Value* scope_lookup_local(Scope* scope, const char* name) {
    for (size_t i = 0; i < scope->count; i++) {
        if (strcmp(scope->names[i], name) == 0) {
            return &scope->values[i];
        }
    }
    return NULL;
}

static Value value_copy(const Value* val);

static void scope_define(Scope* scope, const char* name, Value value) {
    Value* existing = scope_lookup_local(scope, name);
    if (existing) {
        value_free(existing);
        *existing = value_copy(&value);
        return;
    }
    
    if (scope->count >= scope->capacity) {
        scope->capacity *= 2;
        scope->names = (char**)realloc(scope->names, sizeof(char*) * scope->capacity);
        scope->values = (Value*)realloc(scope->values, sizeof(Value) * scope->capacity);
    }
    scope->names[scope->count] = strdup(name);
    scope->values[scope->count] = value_copy(&value);
    scope->count++;
}

/**
 * @brief Creates a new interpreter
 * @return Newly allocated interpreter
 */
Interpreter* interpreter_new(void) {
    Interpreter* interp = (Interpreter*)malloc(sizeof(Interpreter));
    interp->current_scope = scope_create(64, NULL);
    interp->has_return = 0;
    interp->has_break = 0;
    interp->has_continue = 0;
    return interp;
}

/**
 * @brief Sets return value for function
 * @param interp Interpreter
 * @param value Return value
 */
void interpreter_set_return(Interpreter* interp, Value value) {
    interp->return_value = value;
    interp->has_return = 1;
}

/**
 * @brief Gets return value
 * @param interp Interpreter
 * @return Return value
 */
Value interpreter_get_return(Interpreter* interp) {
    return interp->return_value;
}

/**
 * @brief Clears return value
 * @param interp Interpreter
 */
void interpreter_clear_return(Interpreter* interp) {
    interp->has_return = 0;
}

/**
 * @brief Checks if break was encountered
 * @param interp Interpreter
 * @return 1 if break, 0 otherwise
 */
int interpreter_has_break(Interpreter* interp) {
    return interp->has_break;
}

/**
 * @brief Checks if continue was encountered
 * @param interp Interpreter
 * @return 1 if continue, 0 otherwise
 */
int interpreter_has_continue(Interpreter* interp) {
    return interp->has_continue;
}

/**
 * @brief Clears break/continue flags
 * @param interp Interpreter
 */
void interpreter_clear_break_continue(Interpreter* interp) {
    interp->has_break = 0;
    interp->has_continue = 0;
}

/**
 * @brief Frees interpreter
 * @param interp Interpreter to free
 */
void interpreter_free(Interpreter* interp) {
    if (!interp) return;
    scope_free(interp->current_scope);
    free(interp);
}

/** Copies a value (shallow copy) */
static Value value_copy(const Value* val) {
    Value copy;
    copy.type = val->type;
    copy.has_value = val->has_value;
    
    if (val->type == VALUE_STRING) {
        copy.value.string_value = strdup(val->value.string_value);
    } else {
        copy.value = val->value;
    }
    copy.array_length = val->array_length;
    copy.array_elements = val->array_elements;
    return copy;
}

Value value_create_int(int64_t val) {
    Value v;
    v.type = VALUE_INT;
    v.value.int_value = val;
    v.has_value = 1;
    v.array_length = 0;
    v.array_elements = NULL;
    return v;
}

Value value_create_float(double val) {
    Value v;
    v.type = VALUE_FLOAT;
    v.value.float_value = val;
    v.has_value = 1;
    v.array_length = 0;
    v.array_elements = NULL;
    return v;
}

Value value_create_string(char* val) {
    Value v;
    v.type = VALUE_STRING;
    v.value.string_value = strdup(val);
    v.has_value = 1;
    v.array_length = 0;
    v.array_elements = NULL;
    return v;
}

Value value_create_bool(int val) {
    Value v;
    v.type = VALUE_BOOL;
    v.value.bool_value = val;
    v.has_value = 1;
    v.array_length = 0;
    v.array_elements = NULL;
    return v;
}

Value value_create_none(void) {
    Value v;
    v.type = VALUE_NONE;
    v.has_value = 0;
    v.array_length = 0;
    v.array_elements = NULL;
    return v;
}

Value value_create_array(size_t length) {
    Value v;
    v.type = VALUE_ARRAY;
    v.value.int_value = 0;
    v.has_value = 1;
    v.array_length = length;
    v.array_elements = (Value**)calloc(length, sizeof(Value*));
    return v;
}

Value value_create_array_val(Value** arr, size_t length) {
    Value v;
    v.type = VALUE_ARRAY;
    v.value.int_value = 0;
    v.has_value = 1;
    v.array_length = length;
    v.array_elements = arr;
    return v;
}

void value_free(Value* value) {
    if (!value) return;
    if (value->type == VALUE_STRING) {
        free(value->value.string_value);
    } else if (value->type == VALUE_ARRAY) {
        value->array_length = 0;
        value->array_elements = NULL;
    }
    value->type = VALUE_NONE;
}

char* value_to_string(Value* value) {
    if (!value) return strdup("");
    
    char* buf = (char*)malloc(64);
    switch (value->type) {
        case VALUE_INT:
            snprintf(buf, 64, "%ld", (long)value->value.int_value);
            break;
        case VALUE_STRING:
            free(buf);
            return strdup(value->value.string_value);
        case VALUE_BOOL:
            snprintf(buf, 64, "%s", value->value.bool_value ? "true" : "false");
            break;
        case VALUE_NONE:
            snprintf(buf, 64, "nil");
            break;
        default:
            snprintf(buf, 64, "?");
            break;
    }
    return buf;
}

void value_print(Value* value) {
    if (!value) {
        printf("nil");
        return;
    }
    
    switch (value->type) {
        case VALUE_INT:
            printf("%ld", (long)value->value.int_value);
            break;
        case VALUE_FLOAT:
            printf("%g", value->value.float_value);
            break;
        case VALUE_STRING:
            printf("%s", value->value.string_value);
            break;
        case VALUE_BOOL:
            printf("%s", value->value.bool_value ? "true" : "false");
            break;
        case VALUE_NONE:
            printf("nil");
            break;
        case VALUE_ARRAY:
            printf("[");
            for (size_t i = 0; i < value->array_length; i++) {
                if (i > 0) printf(", ");
                value_print(value->array_elements[i]);
            }
            printf("]");
            break;
    }
}

int value_as_bool(Value* value) {
    if (!value || !value->has_value) return 0;
    switch (value->type) {
        case VALUE_BOOL:
            return value->value.bool_value;
        case VALUE_INT:
            return value->value.int_value != 0;
        case VALUE_STRING:
            return value->value.string_value[0] != '\0';
        default:
            return 0;
    }
}

int64_t value_as_int(Value* value) {
    if (!value) return 0;
    if (value->type == VALUE_INT) {
        return value->value.int_value;
    }
    return 0;
}

static Value interpreter_evaluate(Interpreter* interp, ASTNode* node) {
    if (!node) return value_create_none();
    
    switch (node->type) {
        case AST_LITERAL: {
            if (node->data.literal.literal_type == TOKEN_INT_LITERAL) {
                return value_create_int(node->data.literal.value.int_value);
            } else if (node->data.literal.literal_type == TOKEN_FLOAT_LITERAL) {
                return value_create_float(node->data.literal.value.float_value);
            } else if (node->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                return value_create_string(node->data.literal.value.string_value);
            } else if (node->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                return value_create_bool(node->data.literal.value.bool_value);
            }
            return value_create_none();
        }
        
        case AST_IDENTIFIER: {
            char* name = node->data.identifier.name;
            Value* val = scope_lookup(interp->current_scope, name);
            if (val) {
                return value_copy(val);
            }
            fprintf(stderr, "Error: Undefined variable '%s'\n", name);
            return value_create_none();
        }
        
        case AST_BINARY_EXPR: {
            Value left = interpreter_evaluate(interp, node->data.binary_expr.left);
            Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
            TokenType op = node->data.binary_expr.operator;
            Value result = value_create_none();
            
            if (op == TOKEN_PLUS) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value + right.value.int_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    result = value_create_float(left.value.float_value + right.value.float_value);
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    result = value_create_float((double)left.value.int_value + right.value.float_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    result = value_create_float(left.value.float_value + (double)right.value.int_value);
                } else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
                    size_t len = strlen(left.value.string_value) + strlen(right.value.string_value) + 1;
                    char* combined = (char*)malloc(len);
                    strcpy(combined, left.value.string_value);
                    strcat(combined, right.value.string_value);
                    result = value_create_string(combined);
                    free(combined);
                }
            }
            
            if (op == TOKEN_MINUS) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value - right.value.int_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    result = value_create_float(left.value.float_value - right.value.float_value);
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    result = value_create_float((double)left.value.int_value - right.value.float_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    result = value_create_float(left.value.float_value - (double)right.value.int_value);
                }
            }
            
            if (op == TOKEN_STAR) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value * right.value.int_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    result = value_create_float(left.value.float_value * right.value.float_value);
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    result = value_create_float((double)left.value.int_value * right.value.float_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    result = value_create_float(left.value.float_value * (double)right.value.int_value);
                }
            }
            
            if (op == TOKEN_SLASH) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error: Division by zero at runtime\n");
                    } else {
                        result = value_create_int(left.value.int_value / right.value.int_value);
                    }
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    if (right.value.float_value == 0.0) {
                        fprintf(stderr, "Error: Division by zero at runtime\n");
                    } else {
                        result = value_create_float(left.value.float_value / right.value.float_value);
                    }
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    if (right.value.float_value == 0.0) {
                        fprintf(stderr, "Error: Division by zero at runtime\n");
                    } else {
                        result = value_create_float((double)left.value.int_value / right.value.float_value);
                    }
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error: Division by zero at runtime\n");
                    } else {
                        result = value_create_float(left.value.float_value / (double)right.value.int_value);
                    }
                }
            }
            
            if (op == TOKEN_GT) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value > right.value.int_value);
                }
            }
            
            if (op == TOKEN_LT) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value < right.value.int_value);
                }
            }
            
            if (op == TOKEN_GE) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value >= right.value.int_value);
                }
            }
            
            if (op == TOKEN_LE) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value <= right.value.int_value);
                }
            }
            
            if (op == TOKEN_EQ) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value == right.value.int_value);
                }
            }
            
            if (op == TOKEN_NEQ) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value != right.value.int_value);
                }
            }
            
            value_free(&left);
            value_free(&right);
            return result;
        }
        
        case AST_UNARY_EXPR: {
            Value operand = interpreter_evaluate(interp, node->data.unary_expr.operand);
            TokenType op = node->data.unary_expr.operator;
            
            if (op == TOKEN_MINUS && operand.type == VALUE_INT) {
                value_free(&operand);
                return value_create_int(-operand.value.int_value);
            }
            
            if (op == TOKEN_NOT) {
                value_free(&operand);
                return value_create_bool(!value_as_bool(&operand));
            }
            
            value_free(&operand);
            return value_create_none();
        }
        
        case AST_ASSIGN: {
            Value value = interpreter_evaluate(interp, node->data.assign.value);
            scope_define(interp->current_scope, node->data.assign.name, value);
            return value;
        }
        
        case AST_ARRAY_EXPR: {
            Value** elements = (Value**)malloc(sizeof(Value*) * node->data.array_expr.count);
            for (size_t i = 0; i < node->data.array_expr.count; i++) {
                Value* elem = (Value*)malloc(sizeof(Value));
                *elem = interpreter_evaluate(interp, node->data.array_expr.elements[i]);
                elements[i] = elem;
            }
            return value_create_array_val(elements, node->data.array_expr.count);
        }
        
        case AST_INDEX_EXPR: {
            Value array_val = interpreter_evaluate(interp, node->data.index_expr.array);
            Value index_val = interpreter_evaluate(interp, node->data.index_expr.index);
            
            if (array_val.type == VALUE_ARRAY && index_val.type == VALUE_INT) {
                int64_t idx = index_val.value.int_value;
                if (idx >= 0 && (size_t)idx < array_val.array_length) {
                    Value* result = array_val.array_elements[idx];
                    Value copy = value_copy(result);
                    value_free(&array_val);
                    value_free(&index_val);
                    return copy;
                } else {
                    fprintf(stderr, "Error: Index out of bounds (index %ld, length %zu)\n", 
                            (long)idx, array_val.array_length);
                    value_free(&array_val);
                    value_free(&index_val);
                    return value_create_none();
                }
            }
            
            fprintf(stderr, "Error: Can only index into arrays\n");
            value_free(&array_val);
            value_free(&index_val);
            return value_create_none();
        }
        
        case AST_CALL_EXPR: {
            char* name = node->data.call_expr.name;
            
            if (strcmp(name, "len") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int64_t len = 0;
                if (arg.type == VALUE_STRING) {
                    len = (int64_t)strlen(arg.value.string_value);
                } else if (arg.type == VALUE_ARRAY) {
                    len = (int64_t)arg.array_length;
                }
                value_free(&arg);
                return value_create_int(len);
            }
            
            if (strcmp(name, "input") == 0 && node->data.call_expr.arg_count == 0) {
                char buffer[1024];
                if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                    size_t len = strlen(buffer);
                    if (len > 0 && buffer[len - 1] == '\n') {
                        buffer[len - 1] = '\0';
                    }
                    return value_create_string(buffer);
                }
                return value_create_string("");
            }
            
            if (strcmp(name, "to_str") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                char buffer[64];
                if (arg.type == VALUE_INT) {
                    snprintf(buffer, sizeof(buffer), "%ld", (long)arg.value.int_value);
                } else if (arg.type == VALUE_FLOAT) {
                    snprintf(buffer, sizeof(buffer), "%g", arg.value.float_value);
                } else if (arg.type == VALUE_BOOL) {
                    snprintf(buffer, sizeof(buffer), "%s", arg.value.bool_value ? "true" : "false");
                } else if (arg.type == VALUE_STRING) {
                    value_free(&arg);
                    return arg;
                } else {
                    snprintf(buffer, sizeof(buffer), "nil");
                }
                value_free(&arg);
                return value_create_string(buffer);
            }
            
            if (strcmp(name, "to_int") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int64_t result = 0;
                if (arg.type == VALUE_INT) {
                    result = arg.value.int_value;
                } else if (arg.type == VALUE_FLOAT) {
                    result = (int64_t)arg.value.float_value;
                } else if (arg.type == VALUE_STRING) {
                    result = atoll(arg.value.string_value);
                }
                value_free(&arg);
                return value_create_int(result);
            }
            
            if (strcmp(name, "to_float") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                double result = 0.0;
                if (arg.type == VALUE_INT) {
                    result = (double)arg.value.int_value;
                } else if (arg.type == VALUE_FLOAT) {
                    result = arg.value.float_value;
                } else if (arg.type == VALUE_STRING) {
                    result = atof(arg.value.string_value);
                }
                value_free(&arg);
                return value_create_float(result);
            }
            
            fprintf(stderr, "Error: Unknown function '%s'\n", name);
            return value_create_none();
        }
        
        default:
            return value_create_none();
    }
}

static void interpreter_execute_statement(Interpreter* interp, ASTNode* node);

static void interpreter_execute_block_scoped(Interpreter* interp, ASTNode* node) {
    if (!node || node->type != AST_BLOCK) return;
    
    Scope* block_scope = scope_create(16, interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = block_scope;
    
    for (size_t i = 0; i < node->data.block.count; i++) {
        interpreter_execute_statement(interp, node->data.block.statements[i]);
        if (interp->has_break || interp->has_continue || interp->has_return) {
            break;
        }
    }
    
    interp->current_scope = old_scope;
    scope_free(block_scope);
}

static void interpreter_execute_body(Interpreter* interp, ASTNode* body) {
    if (!body) return;
    
    if (body->type == AST_BLOCK) {
        for (size_t i = 0; i < body->data.block.count; i++) {
            interpreter_execute_statement(interp, body->data.block.statements[i]);
            if (interp->has_break || interp->has_continue || interp->has_return) {
                break;
            }
        }
    } else {
        interpreter_execute_statement(interp, body);
    }
}

static void interpreter_execute_statement(Interpreter* interp, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_VAR_DECL: {
            char* name = node->data.var_decl.name;
            ASTNode* init = node->data.var_decl.initializer;
            
            Value value = interpreter_evaluate(interp, init);
            scope_define(interp->current_scope, name, value);
            break;
        }
        
        case AST_BLOCK:
            interpreter_execute_block_scoped(interp, node);
            break;
        
        case AST_FN_DECL:
            if (node->data.fn_decl.body) {
                interpreter_execute_block_scoped(interp, node->data.fn_decl.body);
            }
            break;
        
        case AST_IF_STMT: {
            Value condition = interpreter_evaluate(interp, node->data.if_stmt.condition);
            if (value_as_bool(&condition)) {
                interpreter_execute_statement(interp, node->data.if_stmt.then_branch);
            } else if (node->data.if_stmt.else_branch) {
                interpreter_execute_statement(interp, node->data.if_stmt.else_branch);
            }
            value_free(&condition);
            break;
        }
        
        case AST_PRINT_STMT: {
            Value arg = interpreter_evaluate(interp, node->data.print_stmt.argument);
            value_print(&arg);
            printf("\n");
            value_free(&arg);
            break;
        }
        
        case AST_EXPR_STMT: {
            Value result = interpreter_evaluate(interp, node->data.expr_stmt.expression);
            value_free(&result);
            break;
        }
        
        case AST_WHILE_STMT: {
            while (!interp->has_return && !interp->has_break) {
                Value condition = interpreter_evaluate(interp, node->data.while_stmt.condition);
                if (!value_as_bool(&condition)) {
                    value_free(&condition);
                    break;
                }
                value_free(&condition);
                interpreter_execute_body(interp, node->data.while_stmt.body);
                
                if (interp->has_break) {
                    break;
                }
                
                if (interp->has_continue) {
                    interp->has_continue = 0;
                }
            }
            
            if (interp->has_break) {
                interp->has_break = 0;
            }
            if (interp->has_continue) {
                interp->has_continue = 0;
            }
            break;
        }
        
        case AST_FOR_STMT: {
            if (node->data.for_stmt.initializer) {
                interpreter_execute_statement(interp, node->data.for_stmt.initializer);
            }
            
            while (!interp->has_return && !interp->has_break) {
                if (node->data.for_stmt.condition) {
                    Value condition = interpreter_evaluate(interp, node->data.for_stmt.condition);
                    if (!value_as_bool(&condition)) {
                        value_free(&condition);
                        break;
                    }
                    value_free(&condition);
}
                
                interpreter_execute_body(interp, node->data.for_stmt.body);
                
                if (interp->has_break) {
                    break;
                }
                
                if (interp->has_continue) {
                    interp->has_continue = 0;
                }
                
                if (node->data.for_stmt.update) {
                    Value update_result = interpreter_evaluate(interp, node->data.for_stmt.update);
                    value_free(&update_result);
                }
            }
            
            if (interp->has_break) {
                interp->has_break = 0;
            }
            if (interp->has_continue) {
                interp->has_continue = 0;
            }
            break;
        }
        
        case AST_BREAK_STMT: {
            interp->has_break = 1;
            break;
        }
        
        case AST_CONTINUE_STMT: {
            interp->has_continue = 1;
            break;
        }
        
        case AST_RETURN_STMT: {
            Value return_value;
            if (node->data.return_stmt.value) {
                return_value = interpreter_evaluate(interp, node->data.return_stmt.value);
            } else {
                return_value = value_create_none();
            }
            interpreter_set_return(interp, return_value);
            break;
        }
        
        case AST_ASSIGN: {
            Value value = interpreter_evaluate(interp, node->data.assign.value);
            scope_define(interp->current_scope, node->data.assign.name, value);
            break;
        }
        
        default:
            break;
    }
}

/**
 * @brief Runs the AST program
 * @param interp Interpreter
 * @param program Program AST
 * @return 0 on success
 */
int interpreter_run(Interpreter* interp, ASTNode* program) {
    if (!program || program->type != AST_PROGRAM) {
        return 1;
    }
    
    for (size_t i = 0; i < program->data.program.count; i++) {
        ASTNode* stmt = program->data.program.statements[i];
        
        interpreter_execute_statement(interp, stmt);
    }
    
    return 0;
}