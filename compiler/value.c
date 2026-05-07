#include "value.h"
#include "i18n/i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
