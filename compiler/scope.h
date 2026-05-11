#ifndef HUNNU_SCOPE_H
#define HUNNU_SCOPE_H

#include <stddef.h>
#include "value.h"

typedef struct Scope Scope;

Scope* scope_create(size_t capacity, Scope* parent);
void scope_free(Scope* scope);
Value* scope_lookup(Scope* scope, const char* name);
Value* scope_lookup_local(Scope* scope, const char* name);
void scope_define(Scope* scope, const char* name, Value value, int is_mut);
int scope_is_mutable(Scope* scope, const char* name);

#endif
