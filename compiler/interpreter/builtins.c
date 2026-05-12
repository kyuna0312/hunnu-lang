#include "builtins.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Value builtin_str_to_upper(const char* s) {
    size_t len = strlen(s);
    char* result = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        result[i] = (char)toupper((unsigned char)s[i]);
    }
    result[len] = '\0';
    Value v = value_create_string(result);
    free(result);
    return v;
}

Value builtin_str_to_lower(const char* s) {
    size_t len = strlen(s);
    char* result = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        result[i] = (char)tolower((unsigned char)s[i]);
    }
    result[len] = '\0';
    Value v = value_create_string(result);
    free(result);
    return v;
}

int builtin_str_contains(const char* s, const char* sub) {
    return strstr(s, sub) != NULL;
}

Value builtin_str_trim(const char* s) {
    const char* start = s;
    while (*start && (unsigned char)*start <= ' ') {
        start++;
    }
    if (*start == '\0') {
        return value_create_string("");
    }
    const char* end = start + strlen(start) - 1;
    while (end > start && (unsigned char)*end <= ' ') {
        end--;
    }
    size_t len = (size_t)(end - start + 1);
    char* result = malloc(len + 1);
    memcpy(result, start, len);
    result[len] = '\0';
    Value v = value_create_string(result);
    free(result);
    return v;
}

Value builtin_str_split(const char* s, const char* delim) {
    size_t capacity = 8;
    size_t count = 0;
    Value** elements = malloc(sizeof(Value*) * capacity);

    const char* remaining = s;
    size_t delim_len = strlen(delim);

    while (*remaining) {
        const char* found = strstr(remaining, delim);
        size_t part_len;
        if (found) {
            part_len = (size_t)(found - remaining);
        } else {
            part_len = strlen(remaining);
        }

        if (count >= capacity) {
            capacity *= 2;
            elements = realloc(elements, sizeof(Value*) * capacity);
        }

        char* part = malloc(part_len + 1);
        memcpy(part, remaining, part_len);
        part[part_len] = '\0';
        elements[count] = malloc(sizeof(Value));
        *elements[count] = value_create_string(part);
        free(part);
        count++;

        if (!found) break;
        remaining = found + delim_len;
    }

    if (count == 0) {
        free(elements);
        return value_create_array(0);
    }

    Value result = value_create_array_val(elements, count);
    return result;
}

Value builtin_str_join(Value arr, const char* delim) {
    if (arr.type != VALUE_ARRAY || arr.array_length == 0) {
        return value_create_string("");
    }

    size_t total_len = 0;
    size_t delim_len = strlen(delim);

    for (size_t i = 0; i < arr.array_length; i++) {
        if (arr.array_elements[i]->type == VALUE_STRING) {
            total_len += strlen(arr.array_elements[i]->value.string_value);
        }
        if (i < arr.array_length - 1) {
            total_len += delim_len;
        }
    }

    char* result = malloc(total_len + 1);
    result[0] = '\0';
    char* pos = result;

    for (size_t i = 0; i < arr.array_length; i++) {
        if (arr.array_elements[i]->type == VALUE_STRING) {
            size_t part_len = strlen(arr.array_elements[i]->value.string_value);
            memcpy(pos, arr.array_elements[i]->value.string_value, part_len);
            pos += part_len;
        }
        if (i < arr.array_length - 1) {
            memcpy(pos, delim, delim_len);
            pos += delim_len;
        }
    }
    *pos = '\0';

    Value v = value_create_string(result);
    free(result);
    return v;
}

Value builtin_fs_read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return value_create_string("");
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    if (file_size < 0) {
        fclose(f);
        return value_create_string("");
    }
    rewind(f);

    char* buffer = malloc((size_t)file_size + 1);
    if (!buffer) {
        fclose(f);
        return value_create_string("");
    }

    size_t read_size = fread(buffer, 1, (size_t)file_size, f);
    buffer[read_size] = '\0';
    fclose(f);

    Value v = value_create_string(buffer);
    free(buffer);
    return v;
}

int builtin_fs_write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;

    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, f);
    fclose(f);

    return written == len;
}

Value builtin_arr_push(Value arr, Value val) {
    size_t new_len = arr.array_length + 1;
    Value** new_elements = malloc(sizeof(Value*) * new_len);
    for (size_t i = 0; i < arr.array_length; i++) {
        new_elements[i] = malloc(sizeof(Value));
        *new_elements[i] = value_copy(arr.array_elements[i]);
    }
    new_elements[arr.array_length] = malloc(sizeof(Value));
    *new_elements[arr.array_length] = value_copy(&val);

    Value result = value_create_array(0);
    result.array_elements = new_elements;
    result.array_length = new_len;
    return result;
}

Value builtin_arr_pop(Value arr) {
    if (arr.array_length == 0) {
        return value_create_none();
    }
    return value_copy(arr.array_elements[arr.array_length - 1]);
}
