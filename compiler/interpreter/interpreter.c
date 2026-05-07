/**
 * @file interpreter.c
 * @brief AST interpreter implementation
 */

#include "interpreter.h"
#include "../i18n/i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>

#define MAX_EXTERN_FNS 64

/** External function entry */
typedef struct {
    char* name;
    char* symbol_name;
    void* handle;
    void* func_ptr;
    int returns_int;
    int param_count;
} ExternFn;

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
    int32_t current_line;
    ExternFn extern_fns[MAX_EXTERN_FNS];
    size_t extern_fn_count;
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

Value value_copy(const Value* val);

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
    interp->current_line = 0;
    interp->extern_fn_count = 0;
    memset(interp->extern_fns, 0, sizeof(interp->extern_fns));
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
    for (size_t i = 0; i < interp->extern_fn_count; i++) {
        free(interp->extern_fns[i].name);
        free(interp->extern_fns[i].symbol_name);
        if (interp->extern_fns[i].handle) {
            dlclose(interp->extern_fns[i].handle);
        }
    }
    scope_free(interp->current_scope);
    free(interp);
}

/** Copies a value (deep copy) */
Value value_copy(const Value* val) {
    Value copy;
    copy.type = val->type;
    copy.has_value = val->has_value;
    
    if (val->type == VALUE_STRING) {
        copy.value.string_value = strdup(val->value.string_value);
    } else if (val->type == VALUE_ARRAY) {
        copy.array_length = val->array_length;
        copy.array_elements = (Value**)malloc(sizeof(Value*) * val->array_length);
        for (size_t i = 0; i < val->array_length; i++) {
            copy.array_elements[i] = (Value*)malloc(sizeof(Value));
            *copy.array_elements[i] = value_copy(val->array_elements[i]);
        }
    } else {
        copy.value = val->value;
        copy.array_length = val->array_length;
    }
    if (val->type != VALUE_ARRAY) {
        copy.array_elements = val->array_elements;
    }
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
        for (size_t i = 0; i < value->array_length; i++) {
            value_free(value->array_elements[i]);
            free(value->array_elements[i]);
        }
        free(value->array_elements);
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
            snprintf(buf, 64, "%s", i18n_get_value_string(value->value.bool_value ? "true" : "false"));
            break;
        case VALUE_NONE:
            snprintf(buf, 64, "%s", i18n_get_value_string("nil"));
            break;
        default:
            snprintf(buf, 64, "?");
            break;
    }
    return buf;
}

void value_print(Value* value) {
    if (!value) {
        printf("%s", i18n_get_value_string("nil"));
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
            printf("%s", i18n_get_value_string(value->value.bool_value ? "true" : "false"));
            break;
        case VALUE_NONE:
            printf("%s", i18n_get_value_string("nil"));
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
    
    if (node->line > 0) {
        interp->current_line = node->line;
    }
    
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

        case AST_TYPE_DECL: {
            /* Type declarations are handled at parse time or stored for later use */
            /* For now, just return none - type info is in the AST */
            return value_create_none();
        }

        case AST_FIELD_ACCESS: {
            /* Evaluate the object */
            Value obj = interpreter_evaluate(interp, node->data.field_access.object);
            const char* field = node->data.field_access.field;

            /* Check if it's a struct */
            if (obj.type == VALUE_STRUCT) {
                /* Find the field by name - simplified for now */
                /* In a full implementation, we'd look up field names from type decl */
                value_free(&obj);
                return value_create_none();
            }

            value_free(&obj);
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_UNDEFINED_VARIABLE, "field access on non-struct");
            fprintf(stderr, "\n");
            return value_create_none();
        }

        case AST_ADDRESS_OF: {
            /* For now, return a pointer value */
            Value operand = interpreter_evaluate(interp, node->data.address_of.operand);
            Value result = value_create_none();
            result.type = VALUE_POINTER;
            result.value.pointer_value = malloc(sizeof(Value));
            *(Value*)result.value.pointer_value = operand;
            result.has_value = 1;
            return result;
        }

        case AST_DEREFERENCE: {
            /* Dereference a pointer */
            Value ptr = interpreter_evaluate(interp, node->data.dereference.operand);
            if (ptr.type == VALUE_POINTER && ptr.value.pointer_value) {
                Value result = value_copy((Value*)ptr.value.pointer_value);
                value_free(&ptr);
                return result;
            }
            value_free(&ptr);
            return value_create_none();
        }
        
        case AST_IDENTIFIER: {
            char* name = node->data.identifier.name;
            Value* val = scope_lookup(interp->current_scope, name);
            if (val) {
                return value_copy(val);
            }
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_UNDEFINED_VARIABLE, name);
            fprintf(stderr, "\n");
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
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_int(left.value.int_value / right.value.int_value);
                    }
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    if (right.value.float_value == 0.0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_float(left.value.float_value / right.value.float_value);
                    }
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    if (right.value.float_value == 0.0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_float((double)left.value.int_value / right.value.float_value);
                    }
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
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
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, array_val.array_length);
                    fprintf(stderr, "\n");
                    value_free(&array_val);
                    value_free(&index_val);
                    return value_create_none();
                }
            }
            
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_CAN_ONLY_INDEX_ARRAYS);
            fprintf(stderr, "\n");
            value_free(&array_val);
            value_free(&index_val);
            return value_create_none();
            value_free(&array_val);
            value_free(&index_val);
            return value_create_none();
        }
        
        case AST_INDEX_ASSIGN: {
            Value index_val = interpreter_evaluate(interp, node->data.index_assign.index);
            Value assign_val = interpreter_evaluate(interp, node->data.index_assign.value);
            Value* array_ptr = NULL;
            Value array_val_copy;
            
            if (node->data.index_assign.array->type == AST_IDENTIFIER) {
                array_ptr = scope_lookup(interp->current_scope, node->data.index_assign.array->data.identifier.name);
            }
            
            if (!array_ptr) {
                array_val_copy = interpreter_evaluate(interp, node->data.index_assign.array);
                array_ptr = &array_val_copy;
            }
            
            if (array_ptr->type == VALUE_ARRAY && index_val.type == VALUE_INT) {
                int64_t idx = index_val.value.int_value;
                if (idx >= 0 && (size_t)idx < array_ptr->array_length) {
                    value_free(array_ptr->array_elements[idx]);
                    array_ptr->array_elements[idx] = (Value*)malloc(sizeof(Value));
                    *array_ptr->array_elements[idx] = value_copy(&assign_val);
                    value_free(&index_val);
                    Value result = value_copy(&assign_val);
                    value_free(&assign_val);
                    return result;
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, array_ptr->array_length);
                    fprintf(stderr, "\n");
                    value_free(&index_val);
                    value_free(&assign_val);
                    return value_create_none();
                }
            }
            
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_CAN_ONLY_INDEX_ARRAYS);
            fprintf(stderr, "\n");
            value_free(&index_val);
            value_free(&assign_val);
            return value_create_none();
        }
        
        case AST_CALL_EXPR: {
            char* name = node->data.call_expr.name;
            
            /* Handle 'print' as built-in function */
            if (strcmp(name, "print") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                value_print(&arg);
                printf("\n");
                value_free(&arg);
                return value_create_none();
            }
            
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
                    snprintf(buffer, sizeof(buffer), "%s",
                            i18n_get_value_string(arg.value.bool_value ? "true" : "false"));
                } else if (arg.type == VALUE_STRING) {
                    value_free(&arg);
                    return arg;
                } else {
                    snprintf(buffer, sizeof(buffer), "%s", i18n_get_value_string("nil"));
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
            
            if (strcmp(name, "error") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arg.type != VALUE_STRING) {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    fprintf(stderr, "error() expects a string argument\n");
                    value_free(&arg);
                    return value_create_none();
                }
                fprintf(stderr, "Error: %s\n", arg.value.string_value);
                value_free(&arg);
                return value_create_none();
            }

            if (strcmp(name, "is_error") == 0 || strcmp(name, "unwrap") == 0) {
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "is_error() and unwrap() are not available. Use error() to print errors.\n");
                return value_create_none();
            }

            /* Check for extern function */
            for (size_t i = 0; i < interp->extern_fn_count; i++) {
                if (strcmp(interp->extern_fns[i].name, name) == 0) {
                    ExternFn* ef = &interp->extern_fns[i];
                    
                    if (ef->func_ptr) {
                        /* Build argument array - support int, float, and string */
                        union {
                            int64_t i;
                            double d;
                        } args[8];
                        Value arg_values[8];
                        size_t arg_count = node->data.call_expr.arg_count;
                        int has_float = 0;
                        
                        if (arg_count > 8) {
                            fprintf(stderr, "Error at line %d: ", interp->current_line);
                            i18n_error(ERR_TOO_MANY_ARGS_EXTERN, name);
                            fprintf(stderr, "\n");
                            return value_create_none();
                        }
                        
                        for (size_t j = 0; j < arg_count; j++) {
                            arg_values[j] = interpreter_evaluate(interp, node->data.call_expr.args[j]);
                            if (arg_values[j].type == VALUE_STRING) {
                                args[j].i = (int64_t)(uintptr_t)arg_values[j].value.string_value;
                            } else if (arg_values[j].type == VALUE_FLOAT) {
                                args[j].d = arg_values[j].value.float_value;
                                has_float = 1;
                            } else {
                                args[j].i = (int64_t)arg_values[j].value.int_value;
                            }
                        }
                        
                        /* Call the function based on return type and argument types */
                        if (ef->returns_int == 2) {
                            /* String return type */
                            typedef char* (*str_fn_var)(...);
                            str_fn_var str_func = (str_fn_var)ef->func_ptr;
                            char* str_result = NULL;
                            
                            if (has_float) {
                                switch (arg_count) {
                                    case 0: str_result = ((char*(*)(void))str_func)(); break;
                                    case 1: str_result = ((char*(*)(double))str_func)(args[0].d); break;
                                    case 2: str_result = ((char*(*)(double,double))str_func)(args[0].d, args[1].d); break;
                                    case 3: str_result = ((char*(*)(double,double,double))str_func)(args[0].d, args[1].d, args[2].d); break;
                                    case 4: str_result = ((char*(*)(double,double,double,double))str_func)(args[0].d, args[1].d, args[2].d, args[3].d); break;
                                }
                            } else {
                                switch (arg_count) {
                                    case 0: str_result = ((char*(*)(void))str_func)(); break;
                                    case 1: str_result = ((char*(*)(int64_t))str_func)(args[0].i); break;
                                    case 2: str_result = ((char*(*)(int64_t,int64_t))str_func)(args[0].i, args[1].i); break;
                                    case 3: str_result = ((char*(*)(int64_t,int64_t,int64_t))str_func)(args[0].i, args[1].i, args[2].i); break;
                                    case 4: str_result = ((char*(*)(int64_t,int64_t,int64_t,int64_t))str_func)(args[0].i, args[1].i, args[2].i, args[3].i); break;
                                }
                            }
                            
                            for (size_t j = 0; j < arg_count; j++) value_free(&arg_values[j]);
                            
                            if (str_result) {
                                return value_create_string(str_result);
                            }
                            return value_create_none();
                        } else if (ef->returns_int == 3) {
                            /* Float return type */
                            typedef double (*fn_var)(...);
                            fn_var func = (fn_var)ef->func_ptr;
                            double result = 0.0;
                            
                            if (has_float) {
                                switch (arg_count) {
                                    case 0: result = ((double(*)(void))func)(); break;
                                    case 1: result = ((double(*)(double))func)(args[0].d); break;
                                    case 2: result = ((double(*)(double,double))func)(args[0].d, args[1].d); break;
                                    case 3: result = ((double(*)(double,double,double))func)(args[0].d, args[1].d, args[2].d); break;
                                    case 4: result = ((double(*)(double,double,double,double))func)(args[0].d, args[1].d, args[2].d, args[3].d); break;
                                }
                            } else {
                                switch (arg_count) {
                                    case 0: result = ((double(*)(void))func)(); break;
                                    case 1: result = ((double(*)(int64_t))func)(args[0].i); break;
                                    case 2: result = ((double(*)(int64_t,int64_t))func)(args[0].i, args[1].i); break;
                                    case 3: result = ((double(*)(int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i); break;
                                    case 4: result = ((double(*)(int64_t,int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i, args[3].i); break;
                                }
                            }
                            
                            for (size_t j = 0; j < arg_count; j++) value_free(&arg_values[j]);
                            
                            return value_create_float(result);
                        } else {
                            /* Int/void return type */
                            typedef int64_t (*fn_var)(...);
                            fn_var func = (fn_var)ef->func_ptr;
                            int64_t result = 0;
                            
                            if (has_float) {
                                switch (arg_count) {
                                    case 0: result = ((int64_t(*)(void))func)(); break;
                                    case 1: result = ((int64_t(*)(double))func)(args[0].d); break;
                                    case 2: result = ((int64_t(*)(double,double))func)(args[0].d, args[1].d); break;
                                    case 3: result = ((int64_t(*)(double,double,double))func)(args[0].d, args[1].d, args[2].d); break;
                                    case 4: result = ((int64_t(*)(double,double,double,double))func)(args[0].d, args[1].d, args[2].d, args[3].d); break;
                                }
                            } else {
                                switch (arg_count) {
                                    case 0: result = ((int64_t(*)(void))func)(); break;
                                    case 1: result = ((int64_t(*)(int64_t))func)(args[0].i); break;
                                    case 2: result = ((int64_t(*)(int64_t,int64_t))func)(args[0].i, args[1].i); break;
                                    case 3: result = ((int64_t(*)(int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i); break;
                                    case 4: result = ((int64_t(*)(int64_t,int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i, args[3].i); break;
                                }
                            }
                            
                            for (size_t j = 0; j < arg_count; j++) value_free(&arg_values[j]);
                            
                            return value_create_int(result);
                        }
                    } else {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_EXTERN_NOT_LOADED, name);
                        fprintf(stderr, "\n");
                        return value_create_none();
                    }
                }
            }
            
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_UNKNOWN_FUNCTION, name);
            fprintf(stderr, "\n");
            return value_create_none();
        }
        
        case AST_MATCH_EXPR: {
            Value match_value = interpreter_evaluate(interp, node->data.match_expr.value);
            size_t case_count = node->data.match_expr.case_count;
            
            for (size_t i = 0; i < case_count; i++) {
                ASTNode* pattern = node->data.match_expr.patterns[i];
                ASTNode* body = node->data.match_expr.bodies[i];
                
                int matched = 0;
                
                /* Check if pattern is '_' wildcard */
                if (pattern->type == AST_IDENTIFIER && 
                    strcmp(pattern->data.identifier.name, "_") == 0) {
                    matched = 1;
                }
                /* Check if pattern is a literal */
                else if (pattern->type == AST_LITERAL) {
                    Value pattern_val;
                    if (pattern->data.literal.literal_type == TOKEN_INT_LITERAL) {
                        pattern_val = value_create_int(pattern->data.literal.value.int_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_FLOAT_LITERAL) {
                        pattern_val = value_create_float(pattern->data.literal.value.float_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                        pattern_val = value_create_string(pattern->data.literal.value.string_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                        pattern_val = value_create_bool(pattern->data.literal.value.bool_value);
                    } else {
                        pattern_val = value_create_none();
                    }
                    
                    /* Compare values */
                    if (match_value.type == pattern_val.type) {
                        if (match_value.type == VALUE_INT && 
                            match_value.value.int_value == pattern_val.value.int_value) {
                            matched = 1;
                        } else if (match_value.type == VALUE_FLOAT && 
                                   match_value.value.float_value == pattern_val.value.float_value) {
                            matched = 1;
                        } else if (match_value.type == VALUE_STRING && 
                                   strcmp(match_value.value.string_value, 
                                          pattern_val.value.string_value) == 0) {
                            matched = 1;
                        } else if (match_value.type == VALUE_BOOL && 
                                   match_value.value.bool_value == pattern_val.value.bool_value) {
                            matched = 1;
                        }
                    }
                    value_free(&pattern_val);
                }
                /* Identifier binding pattern */
                else if (pattern->type == AST_IDENTIFIER) {
                    matched = 1;
                }
                
                if (matched) {
                    Value result = interpreter_evaluate(interp, body);
                    value_free(&match_value);
                    return result;
                }
            }
            
            value_free(&match_value);
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
        
        case AST_EXTERN_FN: {
            if (interp->extern_fn_count >= MAX_EXTERN_FNS) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_TOO_MANY_EXTERN_DECLS);
                fprintf(stderr, "\n");
                break;
            }
            
            ExternFn* ef = &interp->extern_fns[interp->extern_fn_count];
            ef->name = strdup(node->data.extern_fn.name);
            ef->symbol_name = strdup(node->data.extern_fn.symbol_name);
            ef->returns_int = node->data.extern_fn.returns_int;
            ef->param_count = (int)node->data.extern_fn.param_count;
            ef->handle = NULL;
            ef->func_ptr = NULL;
            
            /* Load library */
            if (node->data.extern_fn.lib_name) {
                ef->handle = dlopen(node->data.extern_fn.lib_name, RTLD_LAZY);
            } else {
                ef->handle = dlopen(NULL, RTLD_LAZY);
            }
            
            if (!ef->handle) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_CANNOT_LOAD_LIBRARY, dlerror());
                fprintf(stderr, "\n");
                break;
            }
            
            /* Resolve symbol */
            ef->func_ptr = dlsym(ef->handle, ef->symbol_name);
            if (!ef->func_ptr) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_CANNOT_FIND_SYMBOL, ef->symbol_name, dlerror());
                fprintf(stderr, "\n");
                dlclose(ef->handle);
                ef->handle = NULL;
                break;
            }
            
            interp->extern_fn_count++;
            break;
        }
        
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
        
        case AST_MATCH_EXPR: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }
        
        case AST_TRY_STMT: {
            /* Execute try block */
            interpreter_execute_statement(interp, node->data.try_stmt.try_block);
            
            /* Check if an error occurred (simplified - just continue for now) */
            /* TODO: Implement proper error handling with error values */
            
            /* If catch block exists and there was an error, execute it */
            if (node->data.try_stmt.catch_block) {
                /* For now, just execute catch block if present */
                /* In a full implementation, we'd check if an error occurred */
            }
            
            /* Execute finally block if present */
            if (node->data.try_stmt.finally_block) {
                interpreter_execute_statement(interp, node->data.try_stmt.finally_block);
            }
            break;
        }
        
        case AST_INDEX_ASSIGN: {
            Value arr = interpreter_evaluate(interp, node->data.index_assign.array);
            Value idx_val = interpreter_evaluate(interp, node->data.index_assign.index);
            Value val = interpreter_evaluate(interp, node->data.index_assign.value);
            
            if (arr.type == VALUE_ARRAY && idx_val.type == VALUE_INT) {
                int64_t idx = idx_val.value.int_value;
                if (idx >= 0 && (size_t)idx < arr.array_length) {
                    value_free(arr.array_elements[idx]);
                    arr.array_elements[idx] = (Value*)malloc(sizeof(Value));
                    *arr.array_elements[idx] = value_copy(&val);
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, arr.array_length);
                    fprintf(stderr, "\n");
                }
            }
            
            value_free(&arr);
            value_free(&idx_val);
            value_free(&val);
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