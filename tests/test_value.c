#include "../compiler/value.h"
#include "minunit.h"

static int test_value_create_int(void) {
    Value v = value_create_int(42);
    mu_assert_eq(v.type, VALUE_INT, "type should be VALUE_INT");
    mu_assert_eq(v.value.int_value, 42, "value should be 42");
    mu_assert(v.has_value, "has_value should be true");
    value_free(&v);
    return 0;
}

static int test_value_create_float(void) {
    Value v = value_create_float(3.14);
    mu_assert_eq(v.type, VALUE_FLOAT, "type should be VALUE_FLOAT");
    mu_assert(v.value.float_value > 3.13 && v.value.float_value < 3.15, "value should be ~3.14");
    value_free(&v);
    return 0;
}

static int test_value_create_bool(void) {
    Value v = value_create_bool(1);
    mu_assert_eq(v.type, VALUE_BOOL, "type should be VALUE_BOOL");
    mu_assert_eq(v.value.bool_value, 1, "value should be true");
    value_free(&v);
    return 0;
}

static int test_value_create_none(void) {
    Value v = value_create_none();
    mu_assert_eq(v.type, VALUE_NONE, "type should be VALUE_NONE");
    mu_assert(!v.has_value, "has_value should be false");
    value_free(&v);
    return 0;
}

static int test_value_create_string(void) {
    Value v = value_create_string("hello");
    mu_assert_eq(v.type, VALUE_STRING, "type should be VALUE_STRING");
    mu_assert_str_eq(v.value.string_value, "hello", "string value");
    value_free(&v);
    return 0;
}

static int test_value_copy_int(void) {
    Value v = value_create_int(99);
    Value c = value_copy(&v);
    mu_assert_eq(c.type, VALUE_INT, "copied type should be VALUE_INT");
    mu_assert_eq(c.value.int_value, 99, "copied value should be 99");
    value_free(&v);
    value_free(&c);
    return 0;
}

static int test_value_copy_string(void) {
    Value v = value_create_string("copy me");
    Value c = value_copy(&v);
    mu_assert_eq(c.type, VALUE_STRING, "copied type should be VALUE_STRING");
    mu_assert_str_eq(c.value.string_value, "copy me", "copied string value");
    mu_assert(c.value.string_value != v.value.string_value, "should be deep copy");
    value_free(&v);
    value_free(&c);
    return 0;
}

static int test_value_as_bool_int(void) {
    Value t = value_create_int(1);
    Value f = value_create_int(0);
    mu_assert(value_as_bool(&t), "int 1 should be truthy");
    mu_assert(!value_as_bool(&f), "int 0 should be falsy");
    value_free(&t);
    value_free(&f);
    return 0;
}

static int test_value_as_bool_bool(void) {
    Value t = value_create_bool(1);
    Value f = value_create_bool(0);
    mu_assert(value_as_bool(&t), "bool true should be truthy");
    mu_assert(!value_as_bool(&f), "bool false should be falsy");
    value_free(&t);
    value_free(&f);
    return 0;
}

static int test_value_as_bool_none(void) {
    Value n = value_create_none();
    mu_assert(!value_as_bool(&n), "none should be falsy");
    value_free(&n);
    return 0;
}

static int test_value_as_int(void) {
    Value v = value_create_int(123);
    mu_assert_eq(value_as_int(&v), 123, "value_as_int should return 123");
    value_free(&v);
    return 0;
}

static int test_value_create_array(void) {
    Value* e1 = (Value*)malloc(sizeof(Value));
    Value* e2 = (Value*)malloc(sizeof(Value));
    *e1 = value_create_int(10);
    *e2 = value_create_int(20);
    Value** elems = (Value**)malloc(sizeof(Value*) * 2);
    elems[0] = e1;
    elems[1] = e2;
    Value arr = value_create_array_val(elems, 2);
    mu_assert_eq(arr.type, VALUE_ARRAY, "type should be VALUE_ARRAY");
    mu_assert_eq(arr.array_length, 2, "length should be 2");
    mu_assert_eq(arr.array_elements[0]->value.int_value, 10, "first element");
    mu_assert_eq(arr.array_elements[1]->value.int_value, 20, "second element");
    value_free(&arr);
    return 0;
}

void test_suite_value(void) {
    mu_run_test(test_value_create_int);
    mu_run_test(test_value_create_float);
    mu_run_test(test_value_create_bool);
    mu_run_test(test_value_create_none);
    mu_run_test(test_value_create_string);
    mu_run_test(test_value_copy_int);
    mu_run_test(test_value_copy_string);
    mu_run_test(test_value_as_bool_int);
    mu_run_test(test_value_as_bool_bool);
    mu_run_test(test_value_as_bool_none);
    mu_run_test(test_value_as_int);
    mu_run_test(test_value_create_array);
}
