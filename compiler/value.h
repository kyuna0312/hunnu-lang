#ifndef HUNNU_VALUE_H
#define HUNNU_VALUE_H

#include <stdint.h>
#include <stddef.h>

typedef struct Value {
    enum {
        VALUE_INT,
        VALUE_FLOAT,
        VALUE_STRING,
        VALUE_BOOL,
        VALUE_NONE,
        VALUE_ARRAY,
        VALUE_STRUCT,
        VALUE_POINTER
    } type;
    union {
        int64_t int_value;
        double float_value;
        char* string_value;
        int bool_value;
        void* pointer_value;
    } value;
    int has_value;
    size_t array_length;
    struct Value** array_elements;
    char* struct_type;
    struct Value** struct_fields;
    size_t struct_field_count;
} Value;

void value_free(Value* value);
Value value_copy(const Value* value);
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
