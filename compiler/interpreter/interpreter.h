#ifndef HUNNU_INTERPRETER_H
#define HUNNU_INTERPRETER_H

#include "ast/ast.h"

typedef struct Interpreter Interpreter;

Interpreter* interpreter_new(void);
void interpreter_free(Interpreter* interpreter);
int interpreter_run(Interpreter* interpreter, ASTNode* program);

typedef struct Value {
    enum {
        VALUE_INT,
        VALUE_STRING,
        VALUE_BOOL,
        VALUE_NONE,
        VALUE_ARRAY
    } type;
    union {
        int64_t int_value;
        char* string_value;
        int bool_value;
    } value;
    int has_value;
    size_t array_length;
    struct Value** array_elements;
} Value;

void interpreter_set_return(Interpreter* interp, Value value);
Value interpreter_get_return(Interpreter* interp);
void interpreter_clear_return(Interpreter* interp);

Value interpreter_evaluate(Interpreter* interp, ASTNode* node);
void value_free(Value* value);
void value_print(Value* value);
int value_as_bool(Value* value);
int64_t value_as_int(Value* value);

#endif