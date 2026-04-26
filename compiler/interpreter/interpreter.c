#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Interpreter {
    char** names;
    Value* values;
    size_t capacity;
    size_t count;
    Value return_value;
    int has_return;
};

static Value* interpreter_lookup(Interpreter* interp, const char* name) {
    for (size_t i = 0; i < interp->count; i++) {
        if (strcmp(interp->names[i], name) == 0) {
            return &interp->values[i];
        }
    }
    return NULL;
}

static void interpreter_define(Interpreter* interp, const char* name, Value value) {
    Value* existing = interpreter_lookup(interp, name);
    if (existing) {
        if (existing->type == VALUE_STRING) {
            free(existing->value.string_value);
        }
        existing->type = value.type;
        if (value.type == VALUE_STRING) {
            existing->value.string_value = strdup(value.value.string_value);
        } else {
            existing->value = value.value;
        }
        return;
    }
    
    if (interp->count >= interp->capacity) {
        interp->capacity *= 2;
        interp->names = (char**)realloc(interp->names, sizeof(char*) * interp->capacity);
        interp->values = (Value*)realloc(interp->values, sizeof(Value) * interp->capacity);
    }
    interp->names[interp->count] = strdup(name);
    
    interp->values[interp->count].type = value.type;
    if (value.type == VALUE_STRING) {
        interp->values[interp->count].value.string_value = strdup(value.value.string_value);
    } else {
        interp->values[interp->count].value = value.value;
    }
    interp->count++;
}

Interpreter* interpreter_new(void) {
    Interpreter* interp = (Interpreter*)malloc(sizeof(Interpreter));
    interp->capacity = 64;
    interp->count = 0;
    interp->names = (char**)malloc(sizeof(char*) * interp->capacity);
    interp->values = (Value*)malloc(sizeof(Value) * interp->capacity);
    interp->has_return = 0;
    return interp;
}

void interpreter_set_return(Interpreter* interp, Value value) {
    interp->return_value = value;
    interp->has_return = 1;
}

Value interpreter_get_return(Interpreter* interp) {
    return interp->return_value;
}

void interpreter_clear_return(Interpreter* interp) {
    interp->has_return = 0;
}

void interpreter_free(Interpreter* interp) {
    if (!interp) return;
    for (size_t i = 0; i < interp->count; i++) {
        free(interp->names[i]);
        if (interp->values[i].type == VALUE_STRING) {
            free(interp->values[i].value.string_value);
        }
    }
    free(interp->names);
    free(interp->values);
    free(interp);
}

Value value_create_int(int64_t val) {
    Value v;
    v.type = VALUE_INT;
    v.value.int_value = val;
    v.has_value = 1;
    return v;
}

Value value_create_string(char* val) {
    Value v;
    v.type = VALUE_STRING;
    v.value.string_value = strdup(val);
    v.has_value = 1;
    return v;
}

Value value_create_bool(int val) {
    Value v;
    v.type = VALUE_BOOL;
    v.value.bool_value = val;
    v.has_value = 1;
    return v;
}

Value value_create_none(void) {
    Value v;
    v.type = VALUE_NONE;
    v.has_value = 0;
    v.array_length = 0;
    return v;
}

Value value_create_array(Value** arr, size_t length) {
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
    }
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
        case VALUE_STRING:
            printf("%s", value->value.string_value);
            break;
        case VALUE_BOOL:
            printf("%s", value->value.bool_value ? "true" : "false");
            break;
        case VALUE_NONE:
            printf("nil");
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

Value interpreter_evaluate(Interpreter* interp, ASTNode* node) {
    if (!node) return value_create_none();
    
    switch (node->type) {
        case AST_LITERAL: {
            if (node->data.literal.literal_type == TOKEN_INT_LITERAL) {
                return value_create_int(node->data.literal.value.int_value);
            } else if (node->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                return value_create_string(node->data.literal.value.string_value);
            } else if (node->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                return value_create_bool(node->data.literal.value.bool_value);
            }
            return value_create_none();
        }
        
        case AST_IDENTIFIER: {
            char* name = node->data.identifier.name;
            Value* val = interpreter_lookup(interp, name);
            if (val) {
                Value copy;
                copy.type = val->type;
                copy.has_value = val->has_value;
                copy.array_length = val->array_length;
                copy.array_elements = val->array_elements;
                if (val->type == VALUE_STRING) {
                    copy.value.string_value = strdup(val->value.string_value);
                } else {
                    copy.value = val->value;
                }
                return copy;
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
                }
            }
            
            if (op == TOKEN_STAR) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value * right.value.int_value);
                }
            }
            
            if (op == TOKEN_SLASH) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error: Division by zero\n");
                    } else {
                        result = value_create_int(left.value.int_value / right.value.int_value);
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
            interpreter_define(interp, node->data.assign.name, value);
            return value;
        }
        
        case AST_ARRAY_EXPR: {
            Value** elements = (Value**)malloc(sizeof(Value*) * node->data.array_expr.count);
            for (size_t i = 0; i < node->data.array_expr.count; i++) {
                Value* elem = (Value*)malloc(sizeof(Value));
                *elem = interpreter_evaluate(interp, node->data.array_expr.elements[i]);
                elements[i] = elem;
            }
            return value_create_array(elements, node->data.array_expr.count);
        }
        
        case AST_INDEX_EXPR: {
            Value array_val = interpreter_evaluate(interp, node->data.index_expr.array);
            Value index_val = interpreter_evaluate(interp, node->data.index_expr.index);
            
            if (array_val.type == VALUE_ARRAY && index_val.type == VALUE_INT) {
                int64_t idx = index_val.value.int_value;
                if (idx >= 0 && (size_t)idx < array_val.array_length) {
                    Value* result = array_val.array_elements[idx];
                    Value copy;
                    copy.type = result->type;
                    copy.has_value = result->has_value;
                    copy.array_length = result->array_length;
                    copy.array_elements = result->array_elements;
                    if (result->type == VALUE_STRING) {
                        copy.value.string_value = strdup(result->value.string_value);
                    } else {
                        copy.value = result->value;
                    }
                    value_free(&array_val);
                    value_free(&index_val);
                    return copy;
                } else {
                    fprintf(stderr, "Error: Index out of bounds\n");
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
            
            fprintf(stderr, "Error: Unknown function '%s'\n", name);
            return value_create_none();
        }
        
        default:
            return value_create_none();
    }
}

static void interpreter_execute_statement(Interpreter* interp, ASTNode* node);

static void interpreter_execute_block(Interpreter* interp, ASTNode* node) {
    if (!node || node->type != AST_BLOCK) return;
    
    for (size_t i = 0; i < node->data.block.count; i++) {
        interpreter_execute_statement(interp, node->data.block.statements[i]);
    }
}

static void interpreter_execute_statement(Interpreter* interp, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_VAR_DECL: {
            char* name = node->data.var_decl.name;
            ASTNode* init = node->data.var_decl.initializer;
            
            Value value = interpreter_evaluate(interp, init);
            interpreter_define(interp, name, value);
            break;
        }
        
        case AST_BLOCK:
            interpreter_execute_block(interp, node);
            break;
        
        case AST_FN_DECL:
            if (node->data.fn_decl.body) {
                interpreter_execute_block(interp, node->data.fn_decl.body);
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
            while (1) {
                Value condition = interpreter_evaluate(interp, node->data.while_stmt.condition);
                if (!value_as_bool(&condition)) {
                    value_free(&condition);
                    break;
                }
                value_free(&condition);
                interpreter_execute_statement(interp, node->data.while_stmt.body);
                if (interp->has_return) break;
            }
            break;
        }
        
        case AST_FOR_STMT: {
            if (node->data.for_stmt.initializer) {
                interpreter_execute_statement(interp, node->data.for_stmt.initializer);
            }
            while (1) {
                if (node->data.for_stmt.condition) {
                    Value condition = interpreter_evaluate(interp, node->data.for_stmt.condition);
                    if (!value_as_bool(&condition)) {
                        value_free(&condition);
                        break;
                    }
                    value_free(&condition);
                }
                interpreter_execute_statement(interp, node->data.for_stmt.body);
                if (interp->has_return) break;
                if (node->data.for_stmt.update) {
                    interpreter_evaluate(interp, node->data.for_stmt.update);
                }
            }
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
            interpreter_define(interp, node->data.assign.name, value);
            break;
        }
        
        default:
            break;
    }
}

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