#ifndef HUNNU_BUILTINS_H
#define HUNNU_BUILTINS_H

#include "compiler/value.h"

Value builtin_str_to_upper(const char* s);
Value builtin_str_to_lower(const char* s);
int builtin_str_contains(const char* s, const char* sub);
Value builtin_str_trim(const char* s);
Value builtin_str_split(const char* s, const char* delim);
Value builtin_str_join(Value arr, const char* delim);
Value builtin_fs_read_file(const char* path);
int builtin_fs_write_file(const char* path, const char* content);
Value builtin_arr_push(Value arr, Value val);
Value builtin_arr_pop(Value arr);

#endif
