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
        VALUE_FLOAT,
        VALUE_STRING,
        VALUE_BOOL,
        VALUE_NONE,
        VALUE_ARRAY
    } type;
    union {
        int64_t int_value;
        double float_value;
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

int interpreter_has_break(Interpreter* interp);
int interpreter_has_continue(Interpreter* interp);
void interpreter_clear_break_continue(Interpreter* interp);

void value_free(Value* value);
void value_print(Value* value);
int value_as_bool(Value* value);
int64_t value_as_int(Value* value);

Value value_create_int(int64_t val);
Value value_create_float(double val);
Value value_create_string(char* val);
Value value_create_bool(int val);
Value value_create_none(void);
Value value_create_array(size_t length);
Value value_create_array_val(Value** arr, size_t length);
char* value_to_string(Value* value);

#endif