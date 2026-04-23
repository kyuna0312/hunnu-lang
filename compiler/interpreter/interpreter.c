#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Interpreter {
    char** names;
    Value* values;
    size_t capacity;
    size_t count;
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
    return interp;
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
    return v;
}

Value value_create_string(char* val) {
    Value v;
    v.type = VALUE_STRING;
    v.value.string_value = strdup(val);
    return v;
}

Value value_create_bool(int val) {
    Value v;
    v.type = VALUE_BOOL;
    v.value.bool_value = val;
    return v;
}

Value value_create_none(void) {
    Value v;
    v.type = VALUE_NONE;
    return v;
}

void value_free(Value* value) {
    if (!value) return;
    if (value->type == VALUE_STRING) {
        free(value->value.string_value);
    }
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
    if (!value) return 0;
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