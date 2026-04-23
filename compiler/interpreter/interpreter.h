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
        VALUE_NONE
    } type;
    union {
        int64_t int_value;
        char* string_value;
        int bool_value;
    } value;
} Value;

Value interpreter_evaluate(Interpreter* interp, ASTNode* node);
void value_free(Value* value);
void value_print(Value* value);
int value_as_bool(Value* value);
int64_t value_as_int(Value* value);

#endif