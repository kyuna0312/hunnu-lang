#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
    memset(interp->extern_fns, 0, sizeof(interp->extern_fns));
    memset(interp->user_fns, 0, sizeof(interp->user_fns));
    return interp;
}

void interpreter_set_return(Interpreter* interp, Value value) {
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
            interpreter_call_user_fn(interp, &interp->user_fns[i], NULL, 0);
            break;
        }
    }

    return 0;
}
