/**
 * @file value.c
 * @brief Value type system and memory management
 */

#include "value.h"
#include "scope.h"
#include "i18n/i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Value value_copy(const Value* val) {
    Value copy;
    memset(&copy, 0, sizeof(copy));
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
    } else if (val->type == VALUE_STRUCT) {
        copy.struct_type = strdup(val->struct_type);
        copy.struct_field_count = val->struct_field_count;
        copy.struct_fields = (Value**)malloc(sizeof(Value*) * val->struct_field_count);
        for (size_t i = 0; i < val->struct_field_count; i++) {
            copy.struct_fields[i] = (Value*)malloc(sizeof(Value));
            *copy.struct_fields[i] = value_copy(val->struct_fields[i]);
        }
    } else if (val->type == VALUE_ENUM) {
        copy.enum_name = strdup(val->enum_name);
        copy.variant_name = strdup(val->variant_name);
        copy.enum_field_count = val->enum_field_count;
        copy.enum_fields = (Value**)malloc(sizeof(Value*) * val->enum_field_count);
        for (size_t i = 0; i < val->enum_field_count; i++) {
            copy.enum_fields[i] = (Value*)malloc(sizeof(Value));
            *copy.enum_fields[i] = value_copy(val->enum_fields[i]);
        }
    } else if (val->type == VALUE_SYMBOL) {
        copy.value.string_value = strdup(val->value.string_value);
    } else if (val->type == VALUE_OPTION) {
        copy.option_value = val->option_value ? (Value*)malloc(sizeof(Value)) : NULL;
        if (copy.option_value) *copy.option_value = value_copy(val->option_value);
        copy.is_none = val->is_none;
    } else if (val->type == VALUE_RESULT) {
        copy.result_ok = val->result_ok ? (Value*)malloc(sizeof(Value)) : NULL;
        if (copy.result_ok) *copy.result_ok = value_copy(val->result_ok);
        copy.result_err = val->result_err ? (Value*)malloc(sizeof(Value)) : NULL;
        if (copy.result_err) *copy.result_err = value_copy(val->result_err);
    } else if (val->type == VALUE_FUNCTION) {
        copy.fn_decl = val->fn_decl;
        copy.captured_scope = val->captured_scope;
    } else {
        copy.value = val->value;
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
    } else if (value->type == VALUE_STRUCT) {
        free(value->struct_type);
        for (size_t i = 0; i < value->struct_field_count; i++) {
            value_free(value->struct_fields[i]);
            free(value->struct_fields[i]);
        }
        free(value->struct_fields);
        value->struct_field_count = 0;
        value->struct_fields = NULL;
    } else if (value->type == VALUE_ENUM) {
        free(value->enum_name);
        free(value->variant_name);
        for (size_t i = 0; i < value->enum_field_count; i++) {
            value_free(value->enum_fields[i]);
            free(value->enum_fields[i]);
        }
        free(value->enum_fields);
        value->enum_field_count = 0;
        value->enum_fields = NULL;
    } else if (value->type == VALUE_SYMBOL) {
        free(value->value.string_value);
    } else if (value->type == VALUE_OPTION) {
        if (value->option_value) {
            value_free(value->option_value);
            free(value->option_value);
            value->option_value = NULL;
        }
    } else if (value->type == VALUE_RESULT) {
        if (value->result_ok) {
            value_free(value->result_ok);
            free(value->result_ok);
            value->result_ok = NULL;
        }
        if (value->result_err) {
            value_free(value->result_err);
            free(value->result_err);
            value->result_err = NULL;
        }
    } else if (value->type == VALUE_FUNCTION) {
        value->fn_decl = NULL;
        value->captured_scope = NULL;
    } else if (value->type == VALUE_POINTER) {
        if (value->value.pointer_value) {
            Value* ptr_val = (Value*)value->value.pointer_value;
            value_free(ptr_val);
            free(ptr_val);
        }
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
        case VALUE_SYMBOL:
            free(buf);
            buf = (char*)malloc(strlen(value->value.string_value) + 2);
            snprintf(buf, strlen(value->value.string_value) + 2, ":%s", value->value.string_value);
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

        case VALUE_STRUCT:
            printf("%s { ", value->struct_type);
            for (size_t i = 0; i < value->struct_field_count; i++) {
                if (i > 0) printf(", ");
                printf("?");
            }
            printf("}");
            break;

        case VALUE_POINTER:
            printf("ptr");
            break;

        case VALUE_ENUM:
            printf("%s::%s", value->enum_name, value->variant_name);
            if (value->enum_field_count > 0) {
                printf("(");
                for (size_t i = 0; i < value->enum_field_count; i++) {
                    if (i > 0) printf(", ");
                    value_print(value->enum_fields[i]);
                }
                printf(")");
            }
            break;

        case VALUE_SYMBOL:
            printf(":%s", value->value.string_value);
            break;

        case VALUE_OPTION:
            if (value->is_none) {
                printf("None");
            } else if (value->option_value) {
                printf("Some(");
                value_print(value->option_value);
                printf(")");
            }
            break;

        case VALUE_RESULT:
            if (value->result_ok) {
                printf("Ok(");
                value_print(value->result_ok);
                printf(")");
            } else if (value->result_err) {
                printf("Err(");
                value_print(value->result_err);
                printf(")");
            }
            break;

        case VALUE_FUNCTION:
            printf("<fn>");
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
        case VALUE_SYMBOL:
            return 1;
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

Value value_create_struct_value(const char* type_name, Value** fields, size_t field_count) {
    Value v;
    memset(&v, 0, sizeof(v));
    v.type = VALUE_STRUCT;
    v.has_value = 1;
    v.struct_type = strdup(type_name);
    v.struct_fields = fields;
    v.struct_field_count = field_count;
    return v;
}

Value value_create_enum(const char* enum_name, const char* variant_name, Value** fields, size_t field_count) {
    Value v;
    memset(&v, 0, sizeof(v));
    v.type = VALUE_ENUM;
    v.has_value = 1;
    v.enum_name = strdup(enum_name);
    v.variant_name = strdup(variant_name);
    v.enum_fields = fields;
    v.enum_field_count = field_count;
    return v;
}

Value value_create_option(int is_none, Value* inner) {
    Value v;
    memset(&v, 0, sizeof(v));
    v.type = VALUE_OPTION;
    v.has_value = 1;
    v.is_none = is_none;
    v.option_value = inner ? (Value*)malloc(sizeof(Value)) : NULL;
    if (v.option_value && inner) *v.option_value = value_copy(inner);
    return v;
}

Value value_create_result(int is_ok, Value* ok_val, Value* err_val) {
    Value v;
    memset(&v, 0, sizeof(v));
    v.type = VALUE_RESULT;
    v.has_value = 1;
    v.result_ok = ok_val ? (Value*)malloc(sizeof(Value)) : NULL;
    if (v.result_ok && ok_val) *v.result_ok = value_copy(ok_val);
    v.result_err = err_val ? (Value*)malloc(sizeof(Value)) : NULL;
    if (v.result_err && err_val) *v.result_err = value_copy(err_val);
    return v;
}

Value value_create_symbol(const char* name) {
    Value v;
    memset(&v, 0, sizeof(v));
    v.type = VALUE_SYMBOL;
    v.has_value = 1;
    v.value.string_value = strdup(name);
    return v;
}

Value value_create_function(struct ASTNode* fn_decl, struct Scope* captured_scope) {
    Value v;
    memset(&v, 0, sizeof(v));
    v.type = VALUE_FUNCTION;
    v.has_value = 1;
    v.fn_decl = fn_decl;
    v.captured_scope = captured_scope;
    return v;
}
