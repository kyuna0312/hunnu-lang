#include "minunit.h"

int failures = 0;
int passed = 0;

void test_suite_lexer(void);
void test_suite_parser(void);
void test_suite_value(void);
void test_suite_scope(void);
void test_suite_interpreter(void);

int main(void) {
    printf("=== Hunnu C Unit Tests ===\n\n");

    mu_run_suite(test_suite_value);
    mu_run_suite(test_suite_scope);
    mu_run_suite(test_suite_lexer);
    mu_run_suite(test_suite_parser);
    mu_run_suite(test_suite_interpreter);

    printf("=== Results: %d passed, %d failed ===\n", passed, failures);
    return failures;
}
