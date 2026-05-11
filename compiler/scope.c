#include "scope.h"
#include <stdlib.h>
#include <string.h>

struct Scope {
    char** names;
    Value* values;
    int* is_mut;
    size_t capacity;
    size_t count;
    struct Scope* parent;
};

Scope* scope_create(size_t capacity, Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->capacity = capacity;
    scope->count = 0;
    scope->parent = parent;
    scope->names = (char**)malloc(sizeof(char*) * capacity);
    scope->values = (Value*)malloc(sizeof(Value) * capacity);
    scope->is_mut = (int*)malloc(sizeof(int) * capacity);
    return scope;
}

void scope_free(Scope* scope) {
    if (!scope) return;
    for (size_t i = 0; i < scope->count; i++) {
        free(scope->names[i]);
        value_free(&scope->values[i]);
    }
    free(scope->names);
    free(scope->values);
    free(scope->is_mut);
    free(scope);
}

Value* scope_lookup(Scope* scope, const char* name) {
    Scope* current = scope;
    while (current) {
        for (size_t i = 0; i < current->count; i++) {
            if (strcmp(current->names[i], name) == 0) {
                return &current->values[i];
            }
        }
        current = current->parent;
    }
    return NULL;
}

Value* scope_lookup_local(Scope* scope, const char* name) {
    for (size_t i = 0; i < scope->count; i++) {
        if (strcmp(scope->names[i], name) == 0) {
            return &scope->values[i];
        }
    }
    return NULL;
}

void scope_define(Scope* scope, const char* name, Value value, int is_mut) {
    Value* existing = scope_lookup_local(scope, name);
    if (existing) {
        value_free(existing);
        *existing = value_copy(&value);
        size_t idx = (size_t)(existing - scope->values);
        if (idx < scope->count) {
            scope->is_mut[idx] = is_mut;
        }
        return;
    }

    if (scope->count >= scope->capacity) {
        scope->capacity *= 2;
        scope->names = (char**)realloc(scope->names, sizeof(char*) * scope->capacity);
        scope->values = (Value*)realloc(scope->values, sizeof(Value) * scope->capacity);
        scope->is_mut = (int*)realloc(scope->is_mut, sizeof(int) * scope->capacity);
    }
    scope->names[scope->count] = strdup(name);
    scope->values[scope->count] = value_copy(&value);
    scope->is_mut[scope->count] = is_mut;
    scope->count++;
}

int scope_is_mutable(Scope* scope, const char* name) {
    Scope* current = scope;
    while (current) {
        for (size_t i = 0; i < current->count; i++) {
            if (strcmp(current->names[i], name) == 0) {
                return current->is_mut[i];
            }
        }
        current = current->parent;
    }
    return 0;
}
