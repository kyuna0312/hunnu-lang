#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define mu_assert(test, message) do { \
    if (!(test)) { \
        fprintf(stderr, "  FAIL: %s:%d: %s\n", __FILE__, __LINE__, message); \
        return 1; \
    } \
} while (0)

#define mu_assert_eq(actual, expected, message) do { \
    if ((actual) != (expected)) { \
        fprintf(stderr, "  FAIL: %s:%d: %s (got %ld, expected %ld)\n", \
                __FILE__, __LINE__, message, (long)(actual), (long)(expected)); \
        return 1; \
    } \
} while (0)

#define mu_assert_str_eq(actual, expected, message) do { \
    if (strcmp((actual), (expected)) != 0) { \
        fprintf(stderr, "  FAIL: %s:%d: %s (got \"%s\", expected \"%s\")\n", \
                __FILE__, __LINE__, message, (actual), (expected)); \
        return 1; \
    } \
} while (0)

#define mu_run_test(test) do { \
    int ret = test(); \
    if (ret != 0) { \
        fprintf(stderr, "  FAILED: %s\n", #test); \
        failures++; \
    } else { \
        printf("  PASS: %s\n", #test); \
        passed++; \
    } \
} while (0)

#define mu_run_suite(suite) do { \
    printf("--- %s ---\n", #suite); \
    suite(); \
    printf("\n"); \
} while (0)

extern int failures;
extern int passed;

#endif
