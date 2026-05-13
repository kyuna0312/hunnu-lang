/**
 * @file eval.c
 * @brief Expression evaluation for the interpreter
 */

#include "interpreter.h"
#include "../i18n/i18n.h"
#include "builtins.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Value interpreter_evaluate(Interpreter* interp, ASTNode* node) {
    if (!node) return value_create_none();

    if (node->line > 0) {
        interp->current_line = node->line;
    }

    switch (node->type) {
        case AST_LITERAL: {
            if (node->data.literal.literal_type == TOKEN_INT_LITERAL) {
                return value_create_int(node->data.literal.value.int_value);
            } else if (node->data.literal.literal_type == TOKEN_FLOAT_LITERAL) {
                return value_create_float(node->data.literal.value.float_value);
            } else if (node->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                return value_create_string(node->data.literal.value.string_value);
            } else if (node->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                return value_create_bool(node->data.literal.value.bool_value);
            } else if (node->data.literal.literal_type == TOKEN_SYMBOL) {
                return value_create_symbol(node->data.literal.value.string_value);
            }
            return value_create_none();
        }

        case AST_TYPE_DECL: {
            interpreter_register_type(interp, node->data.type_decl.name, NULL,
                                       node->data.type_decl.fields,
                                       node->data.type_decl.is_pub,
                                       node->data.type_decl.field_count);
            return value_create_none();
        }

        case AST_FIELD_ACCESS: {
            Value obj = interpreter_evaluate(interp, node->data.field_access.object);
            const char* field = node->data.field_access.field;

            if (obj.type == VALUE_STRUCT) {
                TypeInfo* t = interpreter_lookup_type(interp, obj.struct_type);
                if (t) {
                    for (size_t i = 0; i < t->field_count; i++) {
                        if (strcmp(t->fields[i], field) == 0) {
                            if (!t->is_pub[i]) {
                                fprintf(stderr, "Warning at line %d: ", interp->current_line);
                                fprintf(stderr, "Accessing private field '%s' of type '%s'\n", field, t->name);
                            }
                            Value result = value_copy(obj.struct_fields[i]);
                            value_free(&obj);
                            return result;
                        }
                    }
                }
                value_free(&obj);
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Field '%s' not found on type '%s'\n", field, obj.struct_type);
                return value_create_none();
            }

            value_free(&obj);
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_UNDEFINED_VARIABLE, "field access on non-struct");
            fprintf(stderr, "\n");
            return value_create_none();
        }

        case AST_ADDRESS_OF: {
            Value operand = interpreter_evaluate(interp, node->data.address_of.operand);
            Value result = value_create_none();
            result.type = VALUE_POINTER;
            result.value.pointer_value = malloc(sizeof(Value));
            *(Value*)result.value.pointer_value = operand;
            result.has_value = 1;
            return result;
        }

        case AST_DEREFERENCE: {
            Value ptr = interpreter_evaluate(interp, node->data.dereference.operand);
            if (ptr.type == VALUE_POINTER && ptr.value.pointer_value) {
                Value result = value_copy((Value*)ptr.value.pointer_value);
                value_free(&ptr);
                return result;
            }
            value_free(&ptr);
            return value_create_none();
        }

        case AST_STRUCT_INSTANCE: {
            TypeInfo* t = interpreter_lookup_type(interp, node->data.struct_instance.type_name);
            if (!t) {
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Unknown type '%s'\n", node->data.struct_instance.type_name);
                return value_create_none();
            }

            Value** fields = (Value**)malloc(sizeof(Value*) * t->field_count);
            for (size_t i = 0; i < t->field_count; i++) {
                fields[i] = (Value*)malloc(sizeof(Value));
                *fields[i] = value_create_none();
            }

            for (size_t i = 0; i < node->data.struct_instance.field_count; i++) {
                const char* fname = node->data.struct_instance.field_names[i];
                int found = 0;
                for (size_t j = 0; j < t->field_count; j++) {
                    if (strcmp(t->fields[j], fname) == 0) {
                        if (!t->is_pub[j]) {
                            fprintf(stderr, "Warning at line %d: ", interp->current_line);
                            fprintf(stderr, "Setting private field '%s' of type '%s'\n", fname, t->name);
                        }
                        value_free(fields[j]);
                        *fields[j] = interpreter_evaluate(interp, node->data.struct_instance.field_values[i]);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    fprintf(stderr, "Unknown field '%s' for type '%s'\n", fname, t->name);
                }
            }

            return value_create_struct_value(t->name, fields, t->field_count);
        }

        case AST_CLASS_DECL: {
            const char* parent_name = node->data.class_decl.parent_name;
            TypeInfo* parent_type = parent_name ? interpreter_lookup_type(interp, parent_name) : NULL;

            char** all_fields = NULL;
            int* all_is_pub = NULL;
            size_t all_field_count = 0;

            if (parent_type) {
                all_field_count = parent_type->field_count + node->data.class_decl.field_count;
                all_fields = (char**)malloc(sizeof(char*) * all_field_count);
                all_is_pub = (int*)malloc(sizeof(int) * all_field_count);
                for (size_t i = 0; i < parent_type->field_count; i++) {
                    all_fields[i] = strdup(parent_type->fields[i]);
                    all_is_pub[i] = parent_type->is_pub[i];
                }
                for (size_t i = 0; i < node->data.class_decl.field_count; i++) {
                    all_fields[parent_type->field_count + i] = strdup(node->data.class_decl.fields[i]);
                    all_is_pub[parent_type->field_count + i] = node->data.class_decl.is_pub[i];
                }
            } else {
                all_field_count = node->data.class_decl.field_count;
                all_fields = (char**)malloc(sizeof(char*) * all_field_count);
                all_is_pub = (int*)malloc(sizeof(int) * all_field_count);
                for (size_t i = 0; i < all_field_count; i++) {
                    all_fields[i] = strdup(node->data.class_decl.fields[i]);
                    all_is_pub[i] = node->data.class_decl.is_pub[i];
                }
            }

            interpreter_register_type(interp, node->data.class_decl.name,
                                       parent_name,
                                       all_fields, all_is_pub, all_field_count);

            if (node->data.class_decl.constructor) {
                ASTNode* ctor = node->data.class_decl.constructor;
                if (interp->user_fn_count < MAX_USER_FNS) {
                    UserFn* ufn = &interp->user_fns[interp->user_fn_count++];
                    char* ctor_name = (char*)malloc(strlen(node->data.class_decl.name) + 5);
                    sprintf(ctor_name, "%s.new", node->data.class_decl.name);
                    ufn->name = ctor_name;
                    ufn->node = ctor;
                }
            }
            for (size_t i = 0; i < node->data.class_decl.method_count; i++) {
                ASTNode* method = node->data.class_decl.methods[i];
                if (interp->user_fn_count < MAX_USER_FNS) {
                    UserFn* ufn = &interp->user_fns[interp->user_fn_count++];
                    ufn->name = strdup(method->data.fn_decl.name);
                    ufn->node = method;
                }
            }

            if (parent_type) {
                for (size_t i = 0; i < interp->user_fn_count; i++) {
                    char* fn_name = interp->user_fns[i].name;
                    size_t plen = strlen(parent_name);
                    if (strncmp(fn_name, parent_name, plen) == 0 && fn_name[plen] == '.') {
                        const char* child_name = node->data.class_decl.name;
                        const char* method_part = fn_name + plen;
                        char* inherited_name = (char*)malloc(strlen(child_name) + strlen(method_part) + 1);
                        sprintf(inherited_name, "%s%s", child_name, method_part);
                        int already_exists = 0;
                        for (size_t j = 0; j < interp->user_fn_count; j++) {
                            if (strcmp(interp->user_fns[j].name, inherited_name) == 0) {
                                already_exists = 1;
                                break;
                            }
                        }
                        if (!already_exists && interp->user_fn_count < MAX_USER_FNS) {
                            UserFn* ufn = &interp->user_fns[interp->user_fn_count++];
                            ufn->name = inherited_name;
                            ufn->node = interp->user_fns[i].node;
                        } else {
                            free(inherited_name);
                        }
                    }
                }
            }

            return value_create_none();
        }

        case AST_NEW_EXPR: {
            const char* cls_name = node->data.new_expr.class_name;
            TypeInfo* t = interpreter_lookup_type(interp, cls_name);
            if (!t) {
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Unknown class '%s'\n", cls_name);
                return value_create_none();
            }
            Value** fields = (Value**)malloc(sizeof(Value*) * t->field_count);
            for (size_t i = 0; i < t->field_count; i++) {
                fields[i] = (Value*)malloc(sizeof(Value));
                if (i < node->data.new_expr.arg_count) {
                    *fields[i] = interpreter_evaluate(interp, node->data.new_expr.args[i]);
                } else {
                    *fields[i] = value_create_none();
                }
            }
            Value instance = value_create_struct_value(t->name, fields, t->field_count);

            char* ctor_name = (char*)malloc(strlen(cls_name) + 5);
            sprintf(ctor_name, "%s.new", cls_name);
            UserFn* ctor_fn = NULL;
            for (size_t i = 0; i < interp->user_fn_count; i++) {
                if (strcmp(interp->user_fns[i].name, ctor_name) == 0) {
                    ctor_fn = &interp->user_fns[i];
                    break;
                }
            }
            free(ctor_name);

            if (ctor_fn) {
                size_t arg_count = node->data.new_expr.arg_count;
                size_t total_args = 1 + arg_count;
                Value* call_args = (Value*)malloc(sizeof(Value) * total_args);
                call_args[0] = value_copy(&instance);
                for (size_t i = 0; i < arg_count; i++) {
                    call_args[1 + i] = interpreter_evaluate(interp, node->data.new_expr.args[i]);
                }
                Value result = interpreter_call_user_fn(interp, ctor_fn, call_args, total_args);
                for (size_t i = 0; i < total_args; i++) value_free(&call_args[i]);
                free(call_args);
                value_free(&result);
            }

            return instance;
        }

        case AST_METHOD_CALL: {
            const char* method = node->data.method_call.method;
            UserFn* ufn = NULL;
            if (node->data.method_call.object->type == AST_IDENTIFIER) {
                const char* obj_name = node->data.method_call.object->data.identifier.name;
                TypeInfo* t = interpreter_lookup_type(interp, obj_name);
                if (t) {
                    size_t mname_len = strlen(t->name) + 1 + strlen(method) + 1;
                    char* mname = (char*)malloc(mname_len);
                    snprintf(mname, mname_len, "%s.%s", t->name, method);
                    TypeInfo* cur_t = t;
                    while (cur_t) {
                        snprintf(mname, mname_len, "%s.%s", cur_t->name, method);
                        for (size_t i = 0; i < interp->user_fn_count; i++) {
                            if (strcmp(interp->user_fns[i].name, mname) == 0) {
                                ufn = &interp->user_fns[i];
                                break;
                            }
                        }
                        if (ufn) break;
                        if (cur_t->parent_name) {
                            cur_t = interpreter_lookup_type(interp, cur_t->parent_name);
                        } else {
                            cur_t = NULL;
                        }
                    }
                    free(mname);
                    if (ufn) {
                        size_t arg_count = node->data.method_call.arg_count;
                        Value* args = (Value*)malloc(sizeof(Value) * arg_count);
                        for (size_t i = 0; i < arg_count; i++) {
                            args[i] = interpreter_evaluate(interp, node->data.method_call.args[i]);
                        }
                        Value result = interpreter_call_user_fn(interp, ufn, args, arg_count);
                        for (size_t i = 0; i < arg_count; i++) value_free(&args[i]);
                        free(args);
                        return result;
                    }
                }
            }

            Value obj = interpreter_evaluate(interp, node->data.method_call.object);
            if (obj.type != VALUE_STRUCT) {
                value_free(&obj);
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Method call on non-struct value\n");
                return value_create_none();
            }

            TypeInfo* cur_t = interpreter_lookup_type(interp, obj.struct_type);
            while (cur_t && !ufn) {
                size_t mname_len = strlen(cur_t->name) + 1 + strlen(method) + 1;
                char* mname = (char*)malloc(mname_len);
                snprintf(mname, mname_len, "%s.%s", cur_t->name, method);
                for (size_t i = 0; i < interp->user_fn_count; i++) {
                    if (strcmp(interp->user_fns[i].name, mname) == 0) {
                        ufn = &interp->user_fns[i];
                        break;
                    }
                }
                free(mname);
                if (ufn) break;
                if (cur_t->parent_name) {
                    cur_t = interpreter_lookup_type(interp, cur_t->parent_name);
                } else {
                    cur_t = NULL;
                }
            }

            if (!ufn) {
                value_free(&obj);
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Method '%s' not found on type '%s'\n", method, obj.struct_type);
                return value_create_none();
            }

            size_t total_args = 1 + node->data.method_call.arg_count;
            Value* call_args = (Value*)malloc(sizeof(Value) * total_args);
            call_args[0] = value_copy(&obj);
            for (size_t i = 0; i < node->data.method_call.arg_count; i++) {
                call_args[1 + i] = interpreter_evaluate(interp, node->data.method_call.args[i]);
            }
            Value result = interpreter_call_user_fn(interp, ufn, call_args, total_args);
            for (size_t i = 0; i < total_args; i++) value_free(&call_args[i]);
            free(call_args);
            value_free(&obj);

            return result;
        }

        case AST_IDENTIFIER: {
            char* name = node->data.identifier.name;
            Value* val = scope_lookup(interp->current_scope, name);
            if (val) {
                return value_copy(val);
            }
            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_UNDEFINED_VARIABLE, name);
            fprintf(stderr, "\n");
            return value_create_none();
        }

        case AST_BINARY_EXPR: {
            TokenType op = node->data.binary_expr.operator;

            if (op == TOKEN_AND) {
                Value left = interpreter_evaluate(interp, node->data.binary_expr.left);
                int left_bool = value_as_bool(&left);
                value_free(&left);
                if (!left_bool) {
                    return value_create_bool(0);
                }
                Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
                int right_bool = value_as_bool(&right);
                value_free(&right);
                return value_create_bool(right_bool);
            }

            if (op == TOKEN_OR) {
                Value left = interpreter_evaluate(interp, node->data.binary_expr.left);
                int left_bool = value_as_bool(&left);
                value_free(&left);
                if (left_bool) {
                    return value_create_bool(1);
                }
                Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
                int right_bool = value_as_bool(&right);
                value_free(&right);
                return value_create_bool(right_bool);
            }

            Value left = interpreter_evaluate(interp, node->data.binary_expr.left);
            Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
            Value result = value_create_none();

            if (op == TOKEN_PLUS) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value + right.value.int_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    result = value_create_float(left.value.float_value + right.value.float_value);
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    result = value_create_float((double)left.value.int_value + right.value.float_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    result = value_create_float(left.value.float_value + (double)right.value.int_value);
                } else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
                    size_t len = strlen(left.value.string_value) + strlen(right.value.string_value) + 1;
                    char* combined = (char*)malloc(len);
                    strcpy(combined, left.value.string_value);
                    strcat(combined, right.value.string_value);
                    result = value_create_string(combined);
                    free(combined);
                }
            }

            if (op == TOKEN_MINUS) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value - right.value.int_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    result = value_create_float(left.value.float_value - right.value.float_value);
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    result = value_create_float((double)left.value.int_value - right.value.float_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    result = value_create_float(left.value.float_value - (double)right.value.int_value);
                }
            }

            if (op == TOKEN_STAR) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_int(left.value.int_value * right.value.int_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    result = value_create_float(left.value.float_value * right.value.float_value);
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    result = value_create_float((double)left.value.int_value * right.value.float_value);
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    result = value_create_float(left.value.float_value * (double)right.value.int_value);
                }
            }

            if (op == TOKEN_SLASH) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_int(left.value.int_value / right.value.int_value);
                    }
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_FLOAT) {
                    if (right.value.float_value == 0.0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_float(left.value.float_value / right.value.float_value);
                    }
                } else if (left.type == VALUE_INT && right.type == VALUE_FLOAT) {
                    if (right.value.float_value == 0.0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_float((double)left.value.int_value / right.value.float_value);
                    }
                } else if (left.type == VALUE_FLOAT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_float(left.value.float_value / (double)right.value.int_value);
                    }
                }
            }

            if (op == TOKEN_PERCENT) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    if (right.value.int_value == 0) {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_DIVISION_BY_ZERO);
                        fprintf(stderr, "\n");
                    } else {
                        result = value_create_int(left.value.int_value % right.value.int_value);
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

            if (op == TOKEN_EQ) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value == right.value.int_value);
                }
            }

            if (op == TOKEN_NEQ) {
                if (left.type == VALUE_INT && right.type == VALUE_INT) {
                    result = value_create_bool(left.value.int_value != right.value.int_value);
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
                int bool_val = value_as_bool(&operand);
                value_free(&operand);
                return value_create_bool(!bool_val);
            }

            value_free(&operand);
            return value_create_none();
        }

        case AST_ASSIGN: {
            Value value = interpreter_evaluate(interp, node->data.assign.value);
            scope_define(interp->current_scope, node->data.assign.name, value);
            return value;
        }

        case AST_ARRAY_EXPR: {
            Value** elements = (Value**)malloc(sizeof(Value*) * node->data.array_expr.count);
            for (size_t i = 0; i < node->data.array_expr.count; i++) {
                Value* elem = (Value*)malloc(sizeof(Value));
                *elem = interpreter_evaluate(interp, node->data.array_expr.elements[i]);
                elements[i] = elem;
            }
            return value_create_array_val(elements, node->data.array_expr.count);
        }

        case AST_INDEX_EXPR: {
            Value array_val = interpreter_evaluate(interp, node->data.index_expr.array);
            Value index_val = interpreter_evaluate(interp, node->data.index_expr.index);

            if (array_val.type == VALUE_ARRAY && index_val.type == VALUE_INT) {
                int64_t idx = index_val.value.int_value;
                if (idx >= 0 && (size_t)idx < array_val.array_length) {
                    Value* result = array_val.array_elements[idx];
                    Value copy = value_copy(result);
                    value_free(&array_val);
                    value_free(&index_val);
                    return copy;
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, array_val.array_length);
                    fprintf(stderr, "\n");
                    value_free(&array_val);
                    value_free(&index_val);
                    return value_create_none();
                }
            }

            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_CAN_ONLY_INDEX_ARRAYS);
            fprintf(stderr, "\n");
            value_free(&array_val);
            value_free(&index_val);
            return value_create_none();
            value_free(&array_val);
            value_free(&index_val);
            return value_create_none();
        }

        case AST_FIELD_ASSIGN: {
            Value obj = interpreter_evaluate(interp, node->data.field_assign.object);
            const char* field = node->data.field_assign.field;
            Value val = interpreter_evaluate(interp, node->data.field_assign.value);

            if (obj.type == VALUE_STRUCT) {
                Value* obj_ptr = NULL;
                if (node->data.field_assign.object->type == AST_IDENTIFIER) {
                    obj_ptr = scope_lookup(interp->current_scope,
                                           node->data.field_assign.object->data.identifier.name);
                }
                Value* target = obj_ptr ? obj_ptr : &obj;
                for (size_t i = 0; i < target->struct_field_count; i++) {
                    TypeInfo* t = interpreter_lookup_type(interp, target->struct_type);
                    if (t && i < t->field_count && strcmp(t->fields[i], field) == 0) {
                        value_free(target->struct_fields[i]);
                        target->struct_fields[i] = (Value*)malloc(sizeof(Value));
                        *target->struct_fields[i] = value_copy(&val);
                        value_free(&obj);
                        value_free(&val);
                        return val;
                    }
                }
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Field '%s' not found on type '%s'\n", field, target->struct_type);
                value_free(&obj);
                value_free(&val);
                return value_create_none();
            }

            fprintf(stderr, "Error at line %d: ", interp->current_line);
            fprintf(stderr, "Field assignment on non-struct\n");
            value_free(&obj);
            value_free(&val);
            return value_create_none();
        }

        case AST_INDEX_ASSIGN: {
            Value index_val = interpreter_evaluate(interp, node->data.index_assign.index);
            Value assign_val = interpreter_evaluate(interp, node->data.index_assign.value);
            Value* array_ptr = NULL;
            Value array_val_copy;

            if (node->data.index_assign.array->type == AST_IDENTIFIER) {
                array_ptr = scope_lookup(interp->current_scope, node->data.index_assign.array->data.identifier.name);
            }

            if (!array_ptr) {
                array_val_copy = interpreter_evaluate(interp, node->data.index_assign.array);
                array_ptr = &array_val_copy;
            }

            if (array_ptr->type == VALUE_ARRAY && index_val.type == VALUE_INT) {
                int64_t idx = index_val.value.int_value;
                if (idx >= 0 && (size_t)idx < array_ptr->array_length) {
                    value_free(array_ptr->array_elements[idx]);
                    array_ptr->array_elements[idx] = (Value*)malloc(sizeof(Value));
                    *array_ptr->array_elements[idx] = value_copy(&assign_val);
                    value_free(&index_val);
                    Value result = value_copy(&assign_val);
                    value_free(&assign_val);
                    return result;
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, array_ptr->array_length);
                    fprintf(stderr, "\n");
                    value_free(&index_val);
                    value_free(&assign_val);
                    return value_create_none();
                }
            }

            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_CAN_ONLY_INDEX_ARRAYS);
            fprintf(stderr, "\n");
            value_free(&index_val);
            value_free(&assign_val);
            return value_create_none();
        }

        case AST_CALL_EXPR: {
            char* name = node->data.call_expr.name;

            if (strcmp(name, "print") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                value_print(&arg);
                printf("\n");
                value_free(&arg);
                return value_create_none();
            }

            if (strcmp(name, "len") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int64_t len = 0;
                if (arg.type == VALUE_STRING) {
                    len = (int64_t)strlen(arg.value.string_value);
                } else if (arg.type == VALUE_ARRAY) {
                    len = (int64_t)arg.array_length;
                }
                value_free(&arg);
                return value_create_int(len);
            }

            if (strcmp(name, "input") == 0 && node->data.call_expr.arg_count == 0) {
                char buffer[1024];
                if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                    size_t len = strlen(buffer);
                    if (len > 0 && buffer[len - 1] == '\n') {
                        buffer[len - 1] = '\0';
                    }
                    return value_create_string(buffer);
                }
                return value_create_string("");
            }

            if (strcmp(name, "to_str") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                char buffer[64];
                if (arg.type == VALUE_INT) {
                    snprintf(buffer, sizeof(buffer), "%ld", (long)arg.value.int_value);
                } else if (arg.type == VALUE_FLOAT) {
                    snprintf(buffer, sizeof(buffer), "%g", arg.value.float_value);
                } else if (arg.type == VALUE_BOOL) {
                    snprintf(buffer, sizeof(buffer), "%s",
                            i18n_get_value_string(arg.value.bool_value ? "true" : "false"));
                } else if (arg.type == VALUE_STRING) {
                    value_free(&arg);
                    return arg;
                } else {
                    snprintf(buffer, sizeof(buffer), "%s", i18n_get_value_string("nil"));
                }
                value_free(&arg);
                return value_create_string(buffer);
            }

            if (strcmp(name, "to_int") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int64_t result = 0;
                if (arg.type == VALUE_INT) {
                    result = arg.value.int_value;
                } else if (arg.type == VALUE_FLOAT) {
                    result = (int64_t)arg.value.float_value;
                } else if (arg.type == VALUE_STRING) {
                    result = atoll(arg.value.string_value);
                }
                value_free(&arg);
                return value_create_int(result);
            }

            if (strcmp(name, "to_float") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                double result = 0.0;
                if (arg.type == VALUE_INT) {
                    result = (double)arg.value.int_value;
                } else if (arg.type == VALUE_FLOAT) {
                    result = arg.value.float_value;
                } else if (arg.type == VALUE_STRING) {
                    result = atof(arg.value.string_value);
                }
                value_free(&arg);
                return value_create_float(result);
            }

            if (strcmp(name, "error") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arg.type != VALUE_STRING) {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    fprintf(stderr, "error() expects a string argument\n");
                    value_free(&arg);
                    return value_create_none();
                }
                fprintf(stderr, "Error: %s\n", arg.value.string_value);
                value_free(&arg);
                return value_create_none();
            }

            if (strcmp(name, "is_error") == 0 || strcmp(name, "unwrap") == 0) {
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "is_error() and unwrap() are not available. Use error() to print errors.\n");
                return value_create_none();
            }

            /* Builtin string operations */
            if (strcmp(name, "__hn_str_to_upper") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arg.type != VALUE_STRING) { value_free(&arg); return value_create_string(""); }
                Value result = builtin_str_to_upper(arg.value.string_value);
                value_free(&arg);
                return result;
            }

            if (strcmp(name, "__hn_str_to_lower") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arg.type != VALUE_STRING) { value_free(&arg); return value_create_string(""); }
                Value result = builtin_str_to_lower(arg.value.string_value);
                value_free(&arg);
                return result;
            }

            if (strcmp(name, "__hn_str_contains") == 0 && node->data.call_expr.arg_count == 2) {
                Value s = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value sub = interpreter_evaluate(interp, node->data.call_expr.args[1]);
                int found = 0;
                if (s.type == VALUE_STRING && sub.type == VALUE_STRING) {
                    found = builtin_str_contains(s.value.string_value, sub.value.string_value);
                }
                value_free(&s);
                value_free(&sub);
                return value_create_bool(found);
            }

            if (strcmp(name, "__hn_str_trim") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arg.type != VALUE_STRING) { value_free(&arg); return value_create_string(""); }
                Value result = builtin_str_trim(arg.value.string_value);
                value_free(&arg);
                return result;
            }

            if (strcmp(name, "__hn_str_split") == 0 && node->data.call_expr.arg_count == 2) {
                Value s = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value delim = interpreter_evaluate(interp, node->data.call_expr.args[1]);
                Value result = value_create_none();
                if (s.type == VALUE_STRING && delim.type == VALUE_STRING) {
                    result = builtin_str_split(s.value.string_value, delim.value.string_value);
                }
                value_free(&s);
                value_free(&delim);
                return result;
            }

            if (strcmp(name, "__hn_str_join") == 0 && node->data.call_expr.arg_count == 2) {
                Value arr = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value delim = interpreter_evaluate(interp, node->data.call_expr.args[1]);
                Value result = value_create_string("");
                if (arr.type == VALUE_ARRAY && delim.type == VALUE_STRING) {
                    result = builtin_str_join(arr, delim.value.string_value);
                }
                value_free(&arr);
                value_free(&delim);
                return result;
            }

            /* Builtin filesystem operations */
            if (strcmp(name, "__hn_fs_read_file") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arg.type != VALUE_STRING) { value_free(&arg); return value_create_string(""); }
                Value result = builtin_fs_read_file(arg.value.string_value);
                value_free(&arg);
                return result;
            }

            if (strcmp(name, "__hn_fs_write_file") == 0 && node->data.call_expr.arg_count == 2) {
                Value path = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value content = interpreter_evaluate(interp, node->data.call_expr.args[1]);
                int ok = 0;
                if (path.type == VALUE_STRING && content.type == VALUE_STRING) {
                    ok = builtin_fs_write_file(path.value.string_value, content.value.string_value);
                }
                value_free(&path);
                value_free(&content);
                return value_create_bool(ok);
            }

            /* Builtin array operations */
            if (strcmp(name, "__hn_arr_push") == 0 && node->data.call_expr.arg_count == 2) {
                Value arr = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value val = interpreter_evaluate(interp, node->data.call_expr.args[1]);
                Value result = arr;
                if (arr.type == VALUE_ARRAY) {
                    result = builtin_arr_push(arr, val);
                }
                value_free(&val);
                return result;
            }

            if (strcmp(name, "__hn_arr_pop") == 0 && node->data.call_expr.arg_count == 1) {
                Value arr = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                if (arr.type != VALUE_ARRAY) {
                    value_free(&arr);
                    return value_create_none();
                }
                Value result = builtin_arr_pop(arr);
                value_free(&arr);
                return result;
            }

            for (size_t i = 0; i < interp->extern_fn_count; i++) {
                if (strcmp(interp->extern_fns[i].name, name) == 0) {
                    ExternFn* ef = &interp->extern_fns[i];

                    if (ef->func_ptr) {
                        union {
                            int64_t i;
                            double d;
                        } args[8];
                        Value arg_values[8];
                        size_t arg_count = node->data.call_expr.arg_count;
                        int has_float = 0;

                        if (arg_count > 8) {
                            fprintf(stderr, "Error at line %d: ", interp->current_line);
                            i18n_error(ERR_TOO_MANY_ARGS_EXTERN, name);
                            fprintf(stderr, "\n");
                            return value_create_none();
                        }

                        for (size_t j = 0; j < arg_count; j++) {
                            arg_values[j] = interpreter_evaluate(interp, node->data.call_expr.args[j]);
                            if (arg_values[j].type == VALUE_STRING) {
                                args[j].i = (int64_t)(uintptr_t)arg_values[j].value.string_value;
                            } else if (arg_values[j].type == VALUE_FLOAT) {
                                args[j].d = arg_values[j].value.float_value;
                                has_float = 1;
                            } else {
                                args[j].i = (int64_t)arg_values[j].value.int_value;
                            }
                        }

                        if (ef->returns_int == 2) {
                            typedef char* (*str_fn_var)(...);
                            str_fn_var str_func = (str_fn_var)ef->func_ptr;
                            char* str_result = NULL;

                            if (has_float) {
                                switch (arg_count) {
                                    case 0: str_result = ((char*(*)(void))str_func)(); break;
                                    case 1: str_result = ((char*(*)(double))str_func)(args[0].d); break;
                                    case 2: str_result = ((char*(*)(double,double))str_func)(args[0].d, args[1].d); break;
                                    case 3: str_result = ((char*(*)(double,double,double))str_func)(args[0].d, args[1].d, args[2].d); break;
                                    case 4: str_result = ((char*(*)(double,double,double,double))str_func)(args[0].d, args[1].d, args[2].d, args[3].d); break;
                                }
                            } else {
                                switch (arg_count) {
                                    case 0: str_result = ((char*(*)(void))str_func)(); break;
                                    case 1: str_result = ((char*(*)(int64_t))str_func)(args[0].i); break;
                                    case 2: str_result = ((char*(*)(int64_t,int64_t))str_func)(args[0].i, args[1].i); break;
                                    case 3: str_result = ((char*(*)(int64_t,int64_t,int64_t))str_func)(args[0].i, args[1].i, args[2].i); break;
                                    case 4: str_result = ((char*(*)(int64_t,int64_t,int64_t,int64_t))str_func)(args[0].i, args[1].i, args[2].i, args[3].i); break;
                                }
                            }

                            for (size_t j = 0; j < arg_count; j++) value_free(&arg_values[j]);

                            if (str_result) {
                                return value_create_string(str_result);
                            }
                            return value_create_none();
                        } else if (ef->returns_int == 3) {
                            typedef double (*fn_var)(...);
                            fn_var func = (fn_var)ef->func_ptr;
                            double result = 0.0;

                            if (has_float) {
                                switch (arg_count) {
                                    case 0: result = ((double(*)(void))func)(); break;
                                    case 1: result = ((double(*)(double))func)(args[0].d); break;
                                    case 2: result = ((double(*)(double,double))func)(args[0].d, args[1].d); break;
                                    case 3: result = ((double(*)(double,double,double))func)(args[0].d, args[1].d, args[2].d); break;
                                    case 4: result = ((double(*)(double,double,double,double))func)(args[0].d, args[1].d, args[2].d, args[3].d); break;
                                }
                            } else {
                                switch (arg_count) {
                                    case 0: result = ((double(*)(void))func)(); break;
                                    case 1: result = ((double(*)(int64_t))func)(args[0].i); break;
                                    case 2: result = ((double(*)(int64_t,int64_t))func)(args[0].i, args[1].i); break;
                                    case 3: result = ((double(*)(int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i); break;
                                    case 4: result = ((double(*)(int64_t,int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i, args[3].i); break;
                                }
                            }

                            for (size_t j = 0; j < arg_count; j++) value_free(&arg_values[j]);

                            return value_create_float(result);
                        } else {
                            typedef int64_t (*fn_var)(...);
                            fn_var func = (fn_var)ef->func_ptr;
                            int64_t result = 0;

                            if (has_float) {
                                switch (arg_count) {
                                    case 0: result = ((int64_t(*)(void))func)(); break;
                                    case 1: result = ((int64_t(*)(double))func)(args[0].d); break;
                                    case 2: result = ((int64_t(*)(double,double))func)(args[0].d, args[1].d); break;
                                    case 3: result = ((int64_t(*)(double,double,double))func)(args[0].d, args[1].d, args[2].d); break;
                                    case 4: result = ((int64_t(*)(double,double,double,double))func)(args[0].d, args[1].d, args[2].d, args[3].d); break;
                                }
                            } else {
                                switch (arg_count) {
                                    case 0: result = ((int64_t(*)(void))func)(); break;
                                    case 1: result = ((int64_t(*)(int64_t))func)(args[0].i); break;
                                    case 2: result = ((int64_t(*)(int64_t,int64_t))func)(args[0].i, args[1].i); break;
                                    case 3: result = ((int64_t(*)(int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i); break;
                                    case 4: result = ((int64_t(*)(int64_t,int64_t,int64_t,int64_t))func)(args[0].i, args[1].i, args[2].i, args[3].i); break;
                                }
                            }

                            for (size_t j = 0; j < arg_count; j++) value_free(&arg_values[j]);

                            return value_create_int(result);
                        }
                    } else {
                        fprintf(stderr, "Error at line %d: ", interp->current_line);
                        i18n_error(ERR_EXTERN_NOT_LOADED, name);
                        fprintf(stderr, "\n");
                        return value_create_none();
                    }
                }
            }

            for (size_t i = 0; i < interp->user_fn_count; i++) {
                if (strcmp(interp->user_fns[i].name, name) == 0) {
                    size_t arg_count = node->data.call_expr.arg_count;
                    Value* args = (Value*)malloc(sizeof(Value) * arg_count);
                    for (size_t j = 0; j < arg_count; j++) {
                        args[j] = interpreter_evaluate(interp, node->data.call_expr.args[j]);
                    }
                    Value result = interpreter_call_user_fn(interp, &interp->user_fns[i], args, arg_count);
                    for (size_t j = 0; j < arg_count; j++) {
                        value_free(&args[j]);
                    }
                    free(args);
                    return result;
                }
            }

            fprintf(stderr, "Error at line %d: ", interp->current_line);
            i18n_error(ERR_UNKNOWN_FUNCTION, name);
            fprintf(stderr, "\n");
            return value_create_none();
        }

        case AST_MATCH_EXPR: {
            Value match_value = interpreter_evaluate(interp, node->data.match_expr.value);
            size_t case_count = node->data.match_expr.case_count;

            for (size_t i = 0; i < case_count; i++) {
                ASTNode* pattern = node->data.match_expr.patterns[i];
                ASTNode* body = node->data.match_expr.bodies[i];

                int matched = 0;

                if (pattern->type == AST_IDENTIFIER &&
                    strcmp(pattern->data.identifier.name, "_") == 0) {
                    matched = 1;
                }
                else if (pattern->type == AST_LITERAL) {
                    Value pattern_val;
                    if (pattern->data.literal.literal_type == TOKEN_INT_LITERAL) {
                        pattern_val = value_create_int(pattern->data.literal.value.int_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_FLOAT_LITERAL) {
                        pattern_val = value_create_float(pattern->data.literal.value.float_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                        pattern_val = value_create_string(pattern->data.literal.value.string_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                        pattern_val = value_create_bool(pattern->data.literal.value.bool_value);
                    } else if (pattern->data.literal.literal_type == TOKEN_SYMBOL) {
                        pattern_val = value_create_symbol(pattern->data.literal.value.string_value);
                    } else {
                        pattern_val = value_create_none();
                    }

                    if (match_value.type == pattern_val.type) {
                        if (match_value.type == VALUE_INT &&
                            match_value.value.int_value == pattern_val.value.int_value) {
                            matched = 1;
                        } else if (match_value.type == VALUE_FLOAT &&
                                   match_value.value.float_value == pattern_val.value.float_value) {
                            matched = 1;
                        } else if (match_value.type == VALUE_STRING &&
                                   strcmp(match_value.value.string_value,
                                          pattern_val.value.string_value) == 0) {
                            matched = 1;
                        } else if (match_value.type == VALUE_BOOL &&
                                   match_value.value.bool_value == pattern_val.value.bool_value) {
                            matched = 1;
                        } else if (match_value.type == VALUE_SYMBOL &&
                                   strcmp(match_value.value.string_value,
                                          pattern_val.value.string_value) == 0) {
                            matched = 1;
                        }
                    }
                    value_free(&pattern_val);
                }
                else if (pattern->type == AST_IDENTIFIER) {
                    matched = 1;
                }

                if (matched) {
                    Value result = interpreter_evaluate(interp, body);
                    value_free(&match_value);
                    return result;
                }
            }

            value_free(&match_value);
            return value_create_none();
        }

        case AST_IMPL_DECL: {
            for (size_t i = 0; i < node->data.impl_decl.method_count; i++) {
                ASTNode* method = node->data.impl_decl.methods[i];
                if (interp->user_fn_count < MAX_USER_FNS) {
                    UserFn* ufn = &interp->user_fns[interp->user_fn_count++];
                    ufn->name = strdup(method->data.fn_decl.name);
                    ufn->node = method;
                }
            }
            return value_create_none();
        }

        case AST_TRAIT_DECL:
            return value_create_none();

        default:
            return value_create_none();
    }
}
