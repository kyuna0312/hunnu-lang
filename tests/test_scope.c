#include "../compiler/scope.h"
#include "../compiler/value.h"
#include "minunit.h"

static int test_scope_define_lookup(void) {
    Scope* s = scope_create(16, NULL);
    Value v = value_create_int(42);
    scope_define(s, "x", v, 1);
    Value* found = scope_lookup(s, "x");
    mu_assert(found != NULL, "should find 'x'");
    mu_assert_eq(found->type, VALUE_INT, "type should be VALUE_INT");
    mu_assert_eq(found->value.int_value, 42, "value should be 42");
    value_free(&v);
    scope_free(s);
    return 0;
}

static int test_scope_lookup_undefined(void) {
    Scope* s = scope_create(16, NULL);
    Value* found = scope_lookup(s, "undefined_var");
    mu_assert(found == NULL, "should not find undefined variable");
    scope_free(s);
    return 0;
}

static int test_scope_nested(void) {
    Scope* outer = scope_create(16, NULL);
    scope_define(outer, "a", value_create_int(1), 1);

    Scope* inner = scope_create(16, outer);
    scope_define(inner, "b", value_create_int(2), 1);

    Value* a = scope_lookup(inner, "a");
    mu_assert(a != NULL, "should find 'a' in outer scope");
    mu_assert_eq(a->value.int_value, 1, "'a' should be 1");

    Value* b = scope_lookup(inner, "b");
    mu_assert(b != NULL, "should find 'b' in inner scope");
    mu_assert_eq(b->value.int_value, 2, "'b' should be 2");

    Value* b_from_outer = scope_lookup(outer, "b");
    mu_assert(b_from_outer == NULL, "should not find 'b' from outer scope");

    scope_free(inner);
    scope_free(outer);
    return 0;
}

static int test_scope_shadowing(void) {
    Scope* outer = scope_create(16, NULL);
    scope_define(outer, "x", value_create_int(1), 1);

    Scope* inner = scope_create(16, outer);
    scope_define(inner, "x", value_create_int(2), 1);

    Value* v = scope_lookup(inner, "x");
    mu_assert_eq(v->value.int_value, 2, "should find shadowed value (2)");

    v = scope_lookup(outer, "x");
    mu_assert_eq(v->value.int_value, 1, "outer scope should still have 1");

    scope_free(inner);
    scope_free(outer);
    return 0;
}

static int test_scope_lookup_local(void) {
    Scope* outer = scope_create(16, NULL);
    scope_define(outer, "x", value_create_int(10), 1);

    Scope* inner = scope_create(16, outer);
    scope_define(inner, "y", value_create_int(20), 1);

    Value* y = scope_lookup_local(inner, "y");
    mu_assert(y != NULL, "should find local 'y'");

    Value* x = scope_lookup_local(inner, "x");
    mu_assert(x == NULL, "should NOT find non-local 'x' via lookup_local");

    scope_free(inner);
    scope_free(outer);
    return 0;
}

static int test_scope_redefine(void) {
    Scope* s = scope_create(16, NULL);
    scope_define(s, "x", value_create_int(1), 1);
    scope_define(s, "x", value_create_int(2), 1);

    Value* v = scope_lookup(s, "x");
    mu_assert_eq(v->value.int_value, 2, "redefined value should be 2");

    scope_free(s);
    return 0;
}

void test_suite_scope(void) {
    mu_run_test(test_scope_define_lookup);
    mu_run_test(test_scope_lookup_undefined);
    mu_run_test(test_scope_nested);
    mu_run_test(test_scope_shadowing);
    mu_run_test(test_scope_lookup_local);
    mu_run_test(test_scope_redefine);
}
