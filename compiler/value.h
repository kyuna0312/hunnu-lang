#ifndef HUNNU_VALUE_H
#define HUNNU_VALUE_H

#include <stdint.h>
#include <stddef.h>

struct ASTNode;
struct Scope;

typedef struct Value {
    enum {
        VALUE_INT,
        VALUE_FLOAT,
        VALUE_STRING,
        VALUE_BOOL,
        VALUE_NONE,
        VALUE_ARRAY,
        VALUE_STRUCT,
        VALUE_POINTER,
        VALUE_ENUM,
        VALUE_FUNCTION
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
    char* enum_name;
    char* variant_name;
    struct Value** enum_fields;
    size_t enum_field_count;
    struct ASTNode* fn_decl;
    struct Scope* captured_scope;
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
Value value_create_struct_value(const char* type_name, Value** fields, size_t field_count);
Value value_create_enum(const char* enum_name, const char* variant_name, Value** fields, size_t field_count);
Value value_create_function(struct ASTNode* fn_decl, struct Scope* captured_scope);
char* value_to_string(Value* value);

#endif
