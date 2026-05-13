/**
 * @file interpreter.c
 * @brief Interpreter lifecycle and state management
 */

#include "interpreter.h"
#include "compiler/value.h"
#include "compiler/scope.h"
#include "compiler/ast/ast.h"
#include "compiler/i18n/i18n.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

Interpreter* interpreter_new(void) {
    Interpreter* interp = (Interpreter*)malloc(sizeof(Interpreter));
    interp->current_scope = scope_create(64, NULL);
    interp->has_return = 0;
    interp->has_break = 0;
    interp->has_continue = 0;
    interp->current_line = 0;
    interp->extern_fn_count = 0;
    interp->type_count = 0;
    interp->type_capacity = 0;
    interp->types = NULL;
    interp->user_fn_count = 0;
    interp->current_fn_name = NULL;
    interp->tco_pending = 0;
    interp->tco_args = NULL;
    interp->tco_arg_count = 0;
    memset(interp->extern_fns, 0, sizeof(interp->extern_fns));
    memset(interp->user_fns, 0, sizeof(interp->user_fns));
    return interp;
}

void interpreter_set_return(Interpreter* interp, Value value) {
    if (interp->has_return) {
        value_free(&interp->return_value);
    }
    interp->return_value = value;
    interp->has_return = 1;
}

Value interpreter_get_return(Interpreter* interp) {
    return interp->return_value;
}

void interpreter_clear_return(Interpreter* interp) {
    interp->has_return = 0;
}

int interpreter_has_break(Interpreter* interp) {
    return interp->has_break;
}

int interpreter_has_continue(Interpreter* interp) {
    return interp->has_continue;
}

void interpreter_clear_break_continue(Interpreter* interp) {
    interp->has_break = 0;
    interp->has_continue = 0;
}

void interpreter_free(Interpreter* interp) {
    if (!interp) return;
    for (size_t i = 0; i < interp->extern_fn_count; i++) {
        free(interp->extern_fns[i].name);
        free(interp->extern_fns[i].symbol_name);
        if (interp->extern_fns[i].handle) {
            dlclose(interp->extern_fns[i].handle);
        }
    }
    for (size_t i = 0; i < interp->type_count; i++) {
        free(interp->types[i].name);
        if (interp->types[i].parent_name) free(interp->types[i].parent_name);
        for (size_t j = 0; j < interp->types[i].field_count; j++) {
            free(interp->types[i].fields[j]);
        }
        free(interp->types[i].fields);
        free(interp->types[i].is_pub);
    }
    free(interp->types);
    for (size_t i = 0; i < interp->user_fn_count; i++) {
        free(interp->user_fns[i].name);
    }
    scope_free(interp->current_scope);
    free(interp);
}

void interpreter_register_type(Interpreter* interp, const char* name, const char* parent_name, char** fields, int* is_pub, size_t field_count) {
    for (size_t i = 0; i < interp->type_count; i++) {
        if (strcmp(interp->types[i].name, name) == 0) {
            return;
        }
    }
    if (interp->type_count >= interp->type_capacity) {
        size_t new_cap = interp->type_capacity == 0 ? 16 : interp->type_capacity * 2;
        interp->types = (TypeInfo*)realloc(interp->types, sizeof(TypeInfo) * new_cap);
        interp->type_capacity = new_cap;
    }
    TypeInfo* t = &interp->types[interp->type_count++];
    t->name = strdup(name);
    t->parent_name = parent_name ? strdup(parent_name) : NULL;
    t->fields = (char**)malloc(sizeof(char*) * field_count);
    t->is_pub = (int*)malloc(sizeof(int) * field_count);
    t->field_count = field_count;
    for (size_t i = 0; i < field_count; i++) {
        t->fields[i] = strdup(fields[i]);
        t->is_pub[i] = is_pub ? is_pub[i] : 1;
    }
}

TypeInfo* interpreter_lookup_type(Interpreter* interp, const char* name) {
    for (size_t i = 0; i < interp->type_count; i++) {
        if (strcmp(interp->types[i].name, name) == 0) {
            return &interp->types[i];
        }
    }
    return NULL;
}

void interpreter_execute_statement(Interpreter* interp, ASTNode* node);
Value interpreter_evaluate(Interpreter* interp, ASTNode* node);

static Value interpreter_call_fn_node(Interpreter* interp, ASTNode* fn_node, Scope* captured_scope, Value* args, size_t arg_count) {
    if (!fn_node || fn_node->type != AST_FN_DECL) {
        return value_create_none();
    }

    ASTNode* body = fn_node->data.fn_decl.body;
    char** params = fn_node->data.fn_decl.params;
    size_t param_count = fn_node->data.fn_decl.param_count;

    if (arg_count != param_count) {
        fprintf(stderr, "Error at line %d: ", interp->current_line);
        fprintf(stderr, "Expected %zu arguments but got %zu\n",
                param_count, arg_count);
        return value_create_none();
    }

    Scope* fn_scope = scope_create(16, captured_scope ? captured_scope : interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = fn_scope;

    for (size_t i = 0; i < arg_count; i++) {
        scope_define(fn_scope, params[i], args[i], 1);
    }

    char* saved_fn_name = interp->current_fn_name;
    if (fn_node->data.fn_decl.name) {
        interp->current_fn_name = fn_node->data.fn_decl.name;
    }

    Value result = value_create_none();

    while (1) {
        interp->has_return = 0;

        if (body) {
            if (body->type == AST_BLOCK) {
                for (size_t i = 0; i < body->data.block.count; i++) {
                    interpreter_execute_statement(interp, body->data.block.statements[i]);
                    if (interp->has_return || interp->tco_pending) break;
                }
            } else {
                interpreter_execute_statement(interp, body);
            }
        }

        if (interp->tco_pending) {
            interp->tco_pending = 0;
            Scope* new_scope = scope_create(16, captured_scope ? captured_scope : old_scope);
            interp->current_scope = new_scope;
            scope_free(fn_scope);
            fn_scope = new_scope;
            for (size_t i = 0; i < interp->tco_arg_count && i < param_count; i++) {
                scope_define(fn_scope, params[i], interp->tco_args[i], 1);
            }
            for (size_t i = 0; i < interp->tco_arg_count; i++) {
                value_free(&interp->tco_args[i]);
            }
            free(interp->tco_args);
            interp->tco_args = NULL;
            interp->tco_arg_count = 0;
            continue;
        }

        if (interp->has_return) {
            result = interp->return_value;
            interp->has_return = 0;
        }
        break;
    }

    interp->current_fn_name = saved_fn_name;
    interp->current_scope = old_scope;
    scope_free(fn_scope);

    return result;
}

static Value interpreter_call_lambda(Interpreter* interp, ASTNode* lambda, Scope* captured_scope, Value* args, size_t arg_count) {
    if (!lambda || lambda->type != AST_LAMBDA) {
        return value_create_none();
    }

    if (arg_count != lambda->data.lambda.param_count) {
        fprintf(stderr, "Error at line %d: ", interp->current_line);
        fprintf(stderr, "Expected %zu arguments but got %zu\n",
                lambda->data.lambda.param_count, arg_count);
        return value_create_none();
    }

    Scope* fn_scope = scope_create(16, captured_scope ? captured_scope : interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = fn_scope;

    for (size_t i = 0; i < arg_count; i++) {
        scope_define(fn_scope, lambda->data.lambda.params[i], args[i], 1);
    }

    interp->has_return = 0;
    Value result = value_create_none();
    ASTNode* body = lambda->data.lambda.body;
    if (body && body->type == AST_BLOCK) {
        for (size_t i = 0; i < body->data.block.count; i++) {
            interpreter_execute_statement(interp, body->data.block.statements[i]);
            if (interp->has_return) break;
        }
        if (interp->has_return) {
            result = interp->return_value;
            interp->has_return = 0;
        }
    } else {
        result = interpreter_evaluate(interp, body);
        if (interp->has_return) {
            result = interp->return_value;
            interp->has_return = 0;
        }
    }

    interp->current_scope = old_scope;
    scope_free(fn_scope);

    return result;
}

Value interpreter_call_user_fn(Interpreter* interp, UserFn* ufn, Value* args, size_t arg_count) {
    if (!ufn || !ufn->node || ufn->node->type != AST_FN_DECL) {
        return value_create_none();
    }

    ASTNode* fn_node = ufn->node;
    ASTNode* body = fn_node->data.fn_decl.body;
    char** params = fn_node->data.fn_decl.params;
    size_t param_count = fn_node->data.fn_decl.param_count;

    /* Check arg count */
    if (arg_count != param_count) {
        fprintf(stderr, "Error at line %d: ", interp->current_line);
        fprintf(stderr, "Expected %zu arguments but got %zu for function '%s'\n",
                param_count, arg_count, ufn->name);
        return value_create_none();
    }

    /* Create new scope for function call */
    Scope* fn_scope = scope_create(16, interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = fn_scope;

    /* Bind parameters */
    for (size_t i = 0; i < arg_count; i++) {
        scope_define(fn_scope, params[i], args[i], 1);
    }

    char* saved_fn_name = interp->current_fn_name;
    interp->current_fn_name = ufn->name;

    Value result = value_create_none();

    /* TCO loop */
    while (1) {
        interp->has_return = 0;

        if (body) {
            if (body->type == AST_BLOCK) {
                for (size_t i = 0; i < body->data.block.count; i++) {
                    interpreter_execute_statement(interp, body->data.block.statements[i]);
                    if (interp->has_return || interp->tco_pending) break;
                }
            } else {
                interpreter_execute_statement(interp, body);
            }
        }

        if (interp->tco_pending) {
            interp->tco_pending = 0;
            Scope* new_scope = scope_create(16, old_scope);
            interp->current_scope = new_scope;
            scope_free(fn_scope);
            fn_scope = new_scope;
            for (size_t i = 0; i < interp->tco_arg_count && i < param_count; i++) {
                scope_define(fn_scope, params[i], interp->tco_args[i], 1);
            }
            for (size_t i = 0; i < interp->tco_arg_count; i++) {
                value_free(&interp->tco_args[i]);
            }
            free(interp->tco_args);
            interp->tco_args = NULL;
            interp->tco_arg_count = 0;
            continue;
        }

        if (interp->has_return) {
            result = interp->return_value;
            interp->has_return = 0;
        }
        break;
    }

    interp->current_fn_name = saved_fn_name;
    interp->current_scope = old_scope;
    scope_free(fn_scope);

    return result;
}

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
            /* For now, return a pointer value */
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
            /* Resolve parent class inheritance */
            const char* parent_name = node->data.class_decl.parent_name;
            TypeInfo* parent_type = parent_name ? interpreter_lookup_type(interp, parent_name) : NULL;

            /* Merge fields: parent fields first, then child fields */
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

            /* Register type with inherited fields */
            interpreter_register_type(interp, node->data.class_decl.name,
                                       parent_name,
                                       all_fields, all_is_pub, all_field_count);

            /* Free merged field arrays (interpreter_register_type strdup'd them) */
            for (size_t i = 0; i < all_field_count; i++) {
                free(all_fields[i]);
            }
            free(all_fields);
            free(all_is_pub);

            /* Register child's own constructor and methods FIRST so they take priority */
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

            /* Inherit parent methods that don't conflict with child's overrides */
            if (parent_type) {
                for (size_t i = 0; i < interp->user_fn_count; i++) {
                    char* fn_name = interp->user_fns[i].name;
                    size_t plen = strlen(parent_name);
                    if (strncmp(fn_name, parent_name, plen) == 0 && fn_name[plen] == '.') {
                        /* Create child version: child_name.method */
                        const char* child_name = node->data.class_decl.name;
                        const char* method_part = fn_name + plen;
                        char* inherited_name = (char*)malloc(strlen(child_name) + strlen(method_part) + 1);
                        sprintf(inherited_name, "%s%s", child_name, method_part);
                        /* Check if child already has this method (skip if overridden) */
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
            /* Create struct instance with fields set from constructor args (by position) */
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

            /* Call constructor if exists for any side effects */
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
            /* Check for static/type method call: TypeName.method() */
            if (node->data.method_call.object->type == AST_IDENTIFIER) {
                const char* obj_name = node->data.method_call.object->data.identifier.name;
                TypeInfo* t = interpreter_lookup_type(interp, obj_name);
                if (t) {
                    size_t mname_len = strlen(t->name) + 1 + strlen(method) + 1;
                    char* mname = (char*)malloc(mname_len);
                    snprintf(mname, mname_len, "%s.%s", t->name, method);
                    /* Walk up inheritance chain for static methods */
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
                        /* Walk up to parent */
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

            /* Instance method call: obj.method() */
            Value obj = interpreter_evaluate(interp, node->data.method_call.object);
            if (obj.type != VALUE_STRUCT) {
                value_free(&obj);
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Method call on non-struct value\n");
                return value_create_none();
            }

            /* Walk up inheritance chain to find method */
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
                /* Walk up to parent */
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
            Value result = value_create_none();

            /* Short-circuit for logical and/or */
            if (op == TOKEN_OR) {
                if (value_as_bool(&left)) {
                    value_free(&left);
                    return value_create_bool(1);
                }
                Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
                result = value_create_bool(value_as_bool(&right));
                value_free(&left);
                value_free(&right);
                return result;
            }

            if (op == TOKEN_AND) {
                if (!value_as_bool(&left)) {
                    value_free(&left);
                    return value_create_bool(0);
                }
                Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
                result = value_create_bool(value_as_bool(&right));
                value_free(&left);
                value_free(&right);
                return result;
            }

            Value right = interpreter_evaluate(interp, node->data.binary_expr.right);
            
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
                        fprintf(stderr, "Division by zero\n");
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
        
        case AST_STRING_CONCAT: {
            Value left = interpreter_evaluate(interp, node->data.string_concat.left);
            Value right = interpreter_evaluate(interp, node->data.string_concat.right);
            char* lstr = value_to_string(&left);
            char* rstr = value_to_string(&right);
            size_t len = strlen(lstr) + strlen(rstr) + 1;
            char* combined = (char*)malloc(len);
            strcpy(combined, lstr);
            strcat(combined, rstr);
            Value result = value_create_string(combined);
            free(combined);
            free(lstr);
            free(rstr);
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
                int result = !value_as_bool(&operand);
                value_free(&operand);
                return value_create_bool(result);
            }
            
            value_free(&operand);
            return value_create_none();
        }
        
        case AST_ASSIGN: {
            if (!scope_is_mutable(interp->current_scope, node->data.assign.name)) {
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Cannot assign to immutable variable '%s'\n", node->data.assign.name);
                return value_create_none();
            }
            Value value = interpreter_evaluate(interp, node->data.assign.value);
            scope_define(interp->current_scope, node->data.assign.name, value, 1);
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
                } else if (idx >= 0 && (size_t)idx == array_ptr->array_length) {
                    /* Auto-extend: append to array */
                    size_t new_len = array_ptr->array_length + 1;
                    array_ptr->array_elements = (Value**)realloc(array_ptr->array_elements, sizeof(Value*) * new_len);
                    array_ptr->array_elements[array_ptr->array_length] = (Value*)malloc(sizeof(Value));
                    *array_ptr->array_elements[array_ptr->array_length] = value_copy(&assign_val);
                    array_ptr->array_length = new_len;
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, array_ptr->array_length);
                    fprintf(stderr, "\n");
                    value_free(&index_val);
                    value_free(&assign_val);
                    return value_create_none();
                }
                value_free(&index_val);
                Value result = value_copy(&assign_val);
                value_free(&assign_val);
                return result;
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
            
            /* Handle 'print' as built-in function */
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

            /* Option/Result constructors */
            if (strcmp(name, "Some") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value result = value_create_option(0, &arg);
                value_free(&arg);
                return result;
            }

            if (strcmp(name, "None") == 0 && node->data.call_expr.arg_count == 0) {
                return value_create_option(1, NULL);
            }

            if (strcmp(name, "Ok") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value result = value_create_result(1, &arg, NULL);
                value_free(&arg);
                return result;
            }

            if (strcmp(name, "Err") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                Value result = value_create_result(0, NULL, &arg);
                value_free(&arg);
                return result;
            }

            /* Option/Result utilities */
            if (strcmp(name, "is_some") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int is_some = (arg.type == VALUE_OPTION && !arg.is_none);
                value_free(&arg);
                return value_create_bool(is_some);
            }

            if (strcmp(name, "is_none") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int is_none = (arg.type == VALUE_OPTION && arg.is_none);
                value_free(&arg);
                return value_create_bool(is_none);
            }

            if (strcmp(name, "is_ok") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int is_ok = (arg.type == VALUE_RESULT && arg.result_ok);
                value_free(&arg);
                return value_create_bool(is_ok);
            }

            if (strcmp(name, "is_err") == 0 && node->data.call_expr.arg_count == 1) {
                Value arg = interpreter_evaluate(interp, node->data.call_expr.args[0]);
                int is_err = (arg.type == VALUE_RESULT && arg.result_err);
                value_free(&arg);
                return value_create_bool(is_err);
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

            /* Check for extern function */
            for (size_t i = 0; i < interp->extern_fn_count; i++) {
                if (strcmp(interp->extern_fns[i].name, name) == 0) {
                    ExternFn* ef = &interp->extern_fns[i];
                    
                    if (ef->func_ptr) {
                        /* Build argument array - support int, float, and string */
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
                        
                        /* Call the function based on return type and argument types */
                        if (ef->returns_int == 2) {
                            /* String return type */
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
                            /* Float return type */
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
                            /* Int/void return type */
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
            
            /* Check for user-defined function */
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

            /* Check for function value in scope */
            Value* fn_val = scope_lookup(interp->current_scope, name);
            if (fn_val && fn_val->type == VALUE_FUNCTION && fn_val->fn_decl) {
                size_t arg_count = node->data.call_expr.arg_count;
                Value* args = (Value*)malloc(sizeof(Value) * arg_count);
                for (size_t j = 0; j < arg_count; j++) {
                    args[j] = interpreter_evaluate(interp, node->data.call_expr.args[j]);
                }
                Value result;
                if (fn_val->fn_decl->type == AST_LAMBDA) {
                    result = interpreter_call_lambda(interp, fn_val->fn_decl, fn_val->captured_scope, args, arg_count);
                } else {
                    result = interpreter_call_fn_node(interp, fn_val->fn_decl, fn_val->captured_scope, args, arg_count);
                }
                for (size_t j = 0; j < arg_count; j++) {
                    value_free(&args[j]);
                }
                free(args);
                return result;
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
                
                /* Check if pattern is '_' wildcard */
                if (pattern->type == AST_IDENTIFIER && 
                    strcmp(pattern->data.identifier.name, "_") == 0) {
                    matched = 1;
                }
                /* Check if pattern is a literal */
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
                    
                    /* Compare values */
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
                /* Identifier binding pattern */
                else if (pattern->type == AST_IDENTIFIER) {
                    matched = 1;
                }
                /* Enum variant pattern: EnumName::Variant(args) */
                else if (pattern->type == AST_ENUM_VARIANT) {
                    int optres_handled = 0;
                    /* Handle VALUE_OPTION pattern matching */
                    if (match_value.type == VALUE_OPTION) {
                        if (strcmp(pattern->data.enum_variant.variant_name, "Some") == 0 && !match_value.is_none) {
                            if (pattern->data.enum_variant.arg_count > 0) {
                                ASTNode* arg = pattern->data.enum_variant.args[0];
                                if (arg && arg->type == AST_IDENTIFIER) {
                                    scope_define(interp->current_scope,
                                                 arg->data.identifier.name,
                                                 value_copy(match_value.option_value), 0);
                                }
                            }
                            matched = 1;
                            optres_handled = 1;
                        } else if (strcmp(pattern->data.enum_variant.variant_name, "None") == 0 && match_value.is_none) {
                            matched = 1;
                            optres_handled = 1;
                        }
                    }
                    /* Handle VALUE_RESULT pattern matching */
                    if (match_value.type == VALUE_RESULT && !optres_handled) {
                        if (strcmp(pattern->data.enum_variant.variant_name, "Ok") == 0 && match_value.result_ok) {
                            if (pattern->data.enum_variant.arg_count > 0) {
                                ASTNode* arg = pattern->data.enum_variant.args[0];
                                if (arg && arg->type == AST_IDENTIFIER) {
                                    scope_define(interp->current_scope,
                                                 arg->data.identifier.name,
                                                 value_copy(match_value.result_ok), 0);
                                }
                            }
                            matched = 1;
                            optres_handled = 1;
                        } else if (strcmp(pattern->data.enum_variant.variant_name, "Err") == 0 && match_value.result_err) {
                            if (pattern->data.enum_variant.arg_count > 0) {
                                ASTNode* arg = pattern->data.enum_variant.args[0];
                                if (arg && arg->type == AST_IDENTIFIER) {
                                    scope_define(interp->current_scope,
                                                 arg->data.identifier.name,
                                                 value_copy(match_value.result_err), 0);
                                }
                            }
                            matched = 1;
                            optres_handled = 1;
                        }
                    }
                    /* Standard VALUE_ENUM pattern matching */
                    if (!optres_handled && match_value.type == VALUE_ENUM &&
                        strcmp(match_value.enum_name, pattern->data.enum_variant.enum_name) == 0 &&
                        strcmp(match_value.variant_name, pattern->data.enum_variant.variant_name) == 0) {
                        if (pattern->data.enum_variant.arg_count > 0 &&
                            match_value.enum_field_count >= pattern->data.enum_variant.arg_count) {
                            for (size_t vi = 0; vi < pattern->data.enum_variant.arg_count; vi++) {
                                ASTNode* arg = pattern->data.enum_variant.args[vi];
                                if (arg && arg->type == AST_IDENTIFIER) {
                                    scope_define(interp->current_scope,
                                                 arg->data.identifier.name,
                                                 value_copy(match_value.enum_fields[vi]), 0);
                                }
                            }
                        }
                        matched = 1;
                    }
                }
                /* Range pattern: start..end */
                else if (pattern->type == AST_RANGE_PATTERN) {
                    Value start_val = interpreter_evaluate(interp, pattern->data.range_pattern.start);
                    Value end_val = interpreter_evaluate(interp, pattern->data.range_pattern.end);
                    if (match_value.type == VALUE_INT &&
                        start_val.type == VALUE_INT && end_val.type == VALUE_INT) {
                        if (match_value.value.int_value >= start_val.value.int_value &&
                            match_value.value.int_value <= end_val.value.int_value) {
                            matched = 1;
                        }
                    } else if (match_value.type == VALUE_FLOAT &&
                               start_val.type == VALUE_FLOAT && end_val.type == VALUE_FLOAT) {
                        if (match_value.value.float_value >= start_val.value.float_value &&
                            match_value.value.float_value <= end_val.value.float_value) {
                            matched = 1;
                        }
                    } else if (match_value.type == VALUE_INT &&
                               start_val.type == VALUE_FLOAT && end_val.type == VALUE_FLOAT) {
                        double mv = (double)match_value.value.int_value;
                        if (mv >= start_val.value.float_value && mv <= end_val.value.float_value) {
                            matched = 1;
                        }
                    } else if (match_value.type == VALUE_FLOAT &&
                               start_val.type == VALUE_INT && end_val.type == VALUE_INT) {
                        if (match_value.value.float_value >= (double)start_val.value.int_value &&
                            match_value.value.float_value <= (double)end_val.value.int_value) {
                            matched = 1;
                        }
                    }
                    value_free(&start_val);
                    value_free(&end_val);
                }
                /* Array destructuring pattern: [a, b, ...rest] */
                else if (pattern->type == AST_ARRAY_PATTERN) {
                    int arr_matches = 0;
                    if (match_value.type == VALUE_ARRAY) {
                        if (pattern->data.array_pattern.rest_name) {
                            arr_matches = match_value.array_length >= pattern->data.array_pattern.count;
                        } else {
                            arr_matches = match_value.array_length == pattern->data.array_pattern.count;
                        }
                    }
                    if (arr_matches) {
                        for (size_t vi = 0; vi < pattern->data.array_pattern.count; vi++) {
                            scope_define(interp->current_scope,
                                         pattern->data.array_pattern.names[vi],
                                         value_copy(match_value.array_elements[vi]), 0);
                        }
                        if (pattern->data.array_pattern.rest_name) {
                            size_t rest_len = match_value.array_length - pattern->data.array_pattern.count;
                            Value** rest_elems = (Value**)malloc(sizeof(Value*) * rest_len);
                            for (size_t vi = 0; vi < rest_len; vi++) {
                                rest_elems[vi] = (Value*)malloc(sizeof(Value));
                                *rest_elems[vi] = value_copy(match_value.array_elements[pattern->data.array_pattern.count + vi]);
                            }
                            Value rest_val = value_create_array_val(rest_elems, rest_len);
                            scope_define(interp->current_scope, pattern->data.array_pattern.rest_name, rest_val, 0);
                            value_free(&rest_val);
                        }
                        matched = 1;
                    }
                }
                
                if (matched) {
                    Value result;
                    if (body->type == AST_BLOCK) {
                        for (size_t bi = 0; bi < body->data.block.count; bi++) {
                            interpreter_execute_statement(interp, body->data.block.statements[bi]);
                            if (interp->has_return) break;
                        }
                        result = value_create_none();
                        if (interp->has_return) {
                            result = interpreter_get_return(interp);
                            interpreter_clear_return(interp);
                        }
                    } else {
                        result = interpreter_evaluate(interp, body);
                    }
                    value_free(&match_value);
                    return result;
                }
            }
            
            value_free(&match_value);
            return value_create_none();
        }
        
        case AST_IMPL_DECL: {
            /* Register trait implementation methods under TypeName.methodName */
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

        case AST_ENUM_VARIANT: {
            /* Create an enum value */
            ASTNode** args = node->data.enum_variant.args;
            size_t arg_count = node->data.enum_variant.arg_count;
            Value** fields = NULL;
            if (arg_count > 0) {
                fields = (Value**)malloc(sizeof(Value*) * arg_count);
                for (size_t i = 0; i < arg_count; i++) {
                    fields[i] = (Value*)malloc(sizeof(Value));
                    *fields[i] = interpreter_evaluate(interp, args[i]);
                }
            }
            return value_create_enum(node->data.enum_variant.enum_name,
                                     node->data.enum_variant.variant_name,
                                     fields, arg_count);
        }

        case AST_ENUM_DECL:
            /* Register enum - for now, just treat as declarative info */
            return value_create_none();

        case AST_LAMBDA:
            return value_create_function(node, interp->current_scope);

        case AST_UNSAFE_BLOCK:
            /* Execute body directly (no extra checks needed in tree-walk) */
            if (node->data.unsafe_block.body) {
                return interpreter_evaluate(interp, node->data.unsafe_block.body);
            }
            return value_create_none();

        default:
            return value_create_none();
    }
}

void interpreter_execute_block_scoped(Interpreter* interp, ASTNode* node) {
    if (!node || node->type != AST_BLOCK) return;
    
    Scope* block_scope = scope_create(16, interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = block_scope;
    
    for (size_t i = 0; i < node->data.block.count; i++) {
        interpreter_execute_statement(interp, node->data.block.statements[i]);
        if (interp->has_break || interp->has_continue || interp->has_return) {
            break;
        }
    }
    
    interp->current_scope = old_scope;
    scope_free(block_scope);
}

void interpreter_execute_body(Interpreter* interp, ASTNode* body) {
    if (!body) return;
    
    if (body->type == AST_BLOCK) {
        for (size_t i = 0; i < body->data.block.count; i++) {
            interpreter_execute_statement(interp, body->data.block.statements[i]);
            if (interp->has_break || interp->has_continue || interp->has_return) {
                break;
            }
        }
    } else {
        interpreter_execute_statement(interp, body);
    }
}

void interpreter_execute_statement(Interpreter* interp, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_VAR_DECL: {
            char* name = node->data.var_decl.name;
            ASTNode* init = node->data.var_decl.initializer;
            
            Value value = interpreter_evaluate(interp, init);
            scope_define(interp->current_scope, name, value, node->data.var_decl.is_mut);
            value_free(&value);
            break;
        }
        
        case AST_BLOCK:
            interpreter_execute_block_scoped(interp, node);
            break;
        
        case AST_FN_DECL: {
            /* Register as a user-defined function instead of executing body */
            if (interp->user_fn_count < MAX_USER_FNS) {
                UserFn* ufn = &interp->user_fns[interp->user_fn_count++];
                ufn->name = strdup(node->data.fn_decl.name);
                ufn->node = node;
            } else {
                fprintf(stderr, "Error at line %d: ", node->line);
                fprintf(stderr, "Too many user-defined functions\n");
            }
            /* Also store in scope as VALUE_FUNCTION for first-class function support */
            Value fn_val = value_create_function(node, NULL);
            scope_define(interp->current_scope, node->data.fn_decl.name, fn_val, 0);
            value_free(&fn_val);
            break;
        }
        
        case AST_EXTERN_FN: {
            if (interp->extern_fn_count >= MAX_EXTERN_FNS) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_TOO_MANY_EXTERN_DECLS);
                fprintf(stderr, "\n");
                break;
            }
            
            /* Load library first to avoid leaking name/symbol_name on error */
            void* handle = NULL;
            if (node->data.extern_fn.lib_name) {
                handle = dlopen(node->data.extern_fn.lib_name, RTLD_LAZY);
            } else {
                handle = dlopen(NULL, RTLD_LAZY);
            }

            if (!handle) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_CANNOT_LOAD_LIBRARY, dlerror());
                fprintf(stderr, "\n");
                break;
            }

            void* func_ptr = dlsym(handle, node->data.extern_fn.symbol_name);
            if (!func_ptr) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_CANNOT_FIND_SYMBOL, node->data.extern_fn.symbol_name, dlerror());
                fprintf(stderr, "\n");
                dlclose(handle);
                break;
            }

            ExternFn* ef = &interp->extern_fns[interp->extern_fn_count];
            ef->name = strdup(node->data.extern_fn.name);
            ef->symbol_name = strdup(node->data.extern_fn.symbol_name);
            ef->returns_int = node->data.extern_fn.returns_int;
            ef->param_count = (int)node->data.extern_fn.param_count;
            ef->handle = handle;
            ef->func_ptr = func_ptr;
            
            interp->extern_fn_count++;
            break;
        }
        
        case AST_IF_STMT: {
            Value condition = interpreter_evaluate(interp, node->data.if_stmt.condition);
            if (value_as_bool(&condition)) {
                interpreter_execute_statement(interp, node->data.if_stmt.then_branch);
            } else if (node->data.if_stmt.else_branch) {
                interpreter_execute_statement(interp, node->data.if_stmt.else_branch);
            }
            value_free(&condition);
            break;
        }
        
        case AST_PRINT_STMT: {
            Value arg = interpreter_evaluate(interp, node->data.print_stmt.argument);
            value_print(&arg);
            printf("\n");
            value_free(&arg);
            break;
        }
        
        case AST_EXPR_STMT: {
            Value result = interpreter_evaluate(interp, node->data.expr_stmt.expression);
            value_free(&result);
            break;
        }
        
        case AST_WHILE_STMT: {
            while (!interp->has_return && !interp->has_break) {
                Value condition = interpreter_evaluate(interp, node->data.while_stmt.condition);
                if (!value_as_bool(&condition)) {
                    value_free(&condition);
                    break;
                }
                value_free(&condition);
                interpreter_execute_body(interp, node->data.while_stmt.body);
                
                if (interp->has_break) {
                    break;
                }
                
                if (interp->has_continue) {
                    interp->has_continue = 0;
                }
            }
            
            if (interp->has_break) {
                interp->has_break = 0;
            }
            if (interp->has_continue) {
                interp->has_continue = 0;
            }
            break;
        }
        
        case AST_FOR_STMT: {
            if (node->data.for_stmt.initializer) {
                interpreter_execute_statement(interp, node->data.for_stmt.initializer);
            }
            
            while (!interp->has_return && !interp->has_break) {
                if (node->data.for_stmt.condition) {
                    Value condition = interpreter_evaluate(interp, node->data.for_stmt.condition);
                    if (!value_as_bool(&condition)) {
                        value_free(&condition);
                        break;
                    }
                    value_free(&condition);
}
                
                interpreter_execute_body(interp, node->data.for_stmt.body);
                
                if (interp->has_break) {
                    break;
                }
                
                if (interp->has_continue) {
                    interp->has_continue = 0;
                }
                
                if (node->data.for_stmt.update) {
                    Value update_result = interpreter_evaluate(interp, node->data.for_stmt.update);
                    value_free(&update_result);
                }
            }
            
            if (interp->has_break) {
                interp->has_break = 0;
            }
            if (interp->has_continue) {
                interp->has_continue = 0;
            }
            break;
        }
        
        case AST_BREAK_STMT: {
            interp->has_break = 1;
            break;
        }
        
        case AST_CONTINUE_STMT: {
            interp->has_continue = 1;
            break;
        }
        
        case AST_RETURN_STMT: {
            /* Detect tail-recursive self-call for TCO */
            if (node->data.return_stmt.value &&
                node->data.return_stmt.value->type == AST_CALL_EXPR &&
                interp->current_fn_name) {
                ASTNode* call = node->data.return_stmt.value;
                char* callee = call->data.call_expr.name;
                if (callee && strcmp(callee, interp->current_fn_name) == 0) {
                    size_t ac = call->data.call_expr.arg_count;
                    Value* tco_args = (Value*)malloc(sizeof(Value) * ac);
                    for (size_t i = 0; i < ac; i++) {
                        tco_args[i] = interpreter_evaluate(interp, call->data.call_expr.args[i]);
                    }
                    interp->tco_args = tco_args;
                    interp->tco_arg_count = ac;
                    interp->tco_pending = 1;
                    break;
                }
            }
            Value return_value;
            if (node->data.return_stmt.value) {
                return_value = interpreter_evaluate(interp, node->data.return_stmt.value);
            } else {
                return_value = value_create_none();
            }
            interpreter_set_return(interp, return_value);
            break;
        }
        
        case AST_ASSIGN: {
            if (!scope_is_mutable(interp->current_scope, node->data.assign.name)) {
                fprintf(stderr, "Error at line %d: ", interp->current_line);
                fprintf(stderr, "Cannot assign to immutable variable '%s'\n", node->data.assign.name);
                break;
            }
            Value value = interpreter_evaluate(interp, node->data.assign.value);
            scope_define(interp->current_scope, node->data.assign.name, value, 1);
            value_free(&value);
            break;
        }
        
        case AST_MATCH_EXPR: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }
        
        case AST_TRY_STMT: {
            /* Execute try block */
            interpreter_execute_statement(interp, node->data.try_stmt.try_block);
            
            /* Check if an error occurred (simplified - just continue for now) */
            /* TODO: Implement proper error handling with error values */
            
            /* If catch block exists and there was an error, execute it */
            if (node->data.try_stmt.catch_block) {
                /* For now, just execute catch block if present */
                /* In a full implementation, we'd check if an error occurred */
            }
            
            /* Execute finally block if present */
            if (node->data.try_stmt.finally_block) {
                interpreter_execute_statement(interp, node->data.try_stmt.finally_block);
            }
            break;
        }
        
        case AST_INDEX_ASSIGN: {
            Value* target_arr = NULL;
            Value arr_copy;
            if (node->data.index_assign.array->type == AST_IDENTIFIER) {
                target_arr = scope_lookup(interp->current_scope, node->data.index_assign.array->data.identifier.name);
            }
            if (!target_arr) {
                arr_copy = interpreter_evaluate(interp, node->data.index_assign.array);
                target_arr = &arr_copy;
            }
            Value idx_val = interpreter_evaluate(interp, node->data.index_assign.index);
            Value val = interpreter_evaluate(interp, node->data.index_assign.value);
            
            if (target_arr->type == VALUE_ARRAY && idx_val.type == VALUE_INT) {
                int64_t idx = idx_val.value.int_value;
                if (idx >= 0 && (size_t)idx < target_arr->array_length) {
                    value_free(target_arr->array_elements[idx]);
                    target_arr->array_elements[idx] = (Value*)malloc(sizeof(Value));
                    *target_arr->array_elements[idx] = value_copy(&val);
                } else if (idx >= 0 && (size_t)idx == target_arr->array_length) {
                    size_t new_len = target_arr->array_length + 1;
                    target_arr->array_elements = (Value**)realloc(target_arr->array_elements, sizeof(Value*) * new_len);
                    target_arr->array_elements[target_arr->array_length] = (Value*)malloc(sizeof(Value));
                    *target_arr->array_elements[target_arr->array_length] = value_copy(&val);
                    target_arr->array_length = new_len;
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, target_arr->array_length);
                    fprintf(stderr, "\n");
                }
            }
            
            value_free(&idx_val);
            value_free(&val);
            break;
        }
        
        case AST_TYPE_DECL:
        case AST_CLASS_DECL:
        case AST_IMPL_DECL:
        case AST_TRAIT_DECL:
        case AST_ENUM_DECL: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }

        case AST_STRUCT_INSTANCE:
        case AST_METHOD_CALL:
        case AST_FIELD_ACCESS:
        case AST_FIELD_ASSIGN:
        case AST_ADDRESS_OF:
        case AST_DEREFERENCE:
        case AST_NEW_EXPR:
        case AST_ENUM_VARIANT: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }

        case AST_UNSAFE_BLOCK: {
            ASTNode* body = node->data.unsafe_block.body;
            if (body && body->type == AST_BLOCK) {
                for (size_t bi = 0; bi < body->data.block.count; bi++) {
                    interpreter_execute_statement(interp, body->data.block.statements[bi]);
                    if (interp->has_return || interp->has_break) break;
                }
            }
            break;
        }

        default:
            break;
    }
}

/**
 * @brief Runs the AST program
 * @param interp Interpreter
 * @param program Program AST
 * @return 0 on success
 */
int interpreter_run(Interpreter* interp, ASTNode* program) {
    if (!program || program->type != AST_PROGRAM) {
        return 1;
    }

    for (size_t i = 0; i < program->data.program.count; i++) {
        ASTNode* stmt = program->data.program.statements[i];

        interpreter_execute_statement(interp, stmt);
    }

    for (size_t i = 0; i < interp->user_fn_count; i++) {
        if (strcmp(interp->user_fns[i].name, "main") == 0) {
            Value main_result = interpreter_call_user_fn(interp, &interp->user_fns[i], NULL, 0);
            value_free(&main_result);
            break;
        }
    }

    return 0;
}
