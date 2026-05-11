#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/interpreter/interpreter.h"
#include "../compiler/ast/ast.h"
#include "minunit.h"

static ASTNode* parse_source(const char* source) {
    Lexer* lexer = lexer_new(source);
    ASTNode* ast = parse(lexer);
    lexer_free(lexer);
    return ast;
}

static int test_interpreter_arithmetic(void) {
    ASTNode* prog = parse_source("fn main() { let x = 2 + 3; print(x) }");
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_fn_call(void) {
    ASTNode* prog = parse_source(
        "fn add(a, b) { return a + b }\n"
        "fn main() { let r = add(3, 4); print(r) }"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_if_else(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let x = 10\n"
        "  if x > 5 { print(1) } else { print(2) }\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_while_loop(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let mut x = 0\n"
        "  while x < 3 { x = x + 1 }\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_recursion(void) {
    ASTNode* prog = parse_source(
        "fn fact(n) {\n"
        "  if n <= 1 { return 1 }\n"
        "  return n * fact(n - 1)\n"
        "}\n"
        "fn main() { let r = fact(5); print(r) }"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_nested_scopes(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let x = 1\n"
        "  { let x = 2; print(x) }\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_bool_logic(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let t = true\n"
        "  let f = false\n"
        "  if t { print(1) }\n"
        "  if !f { print(2) }\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_string_concat(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let s = \"hello\" + \" world\"\n"
        "  print(s)\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_array_index(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let arr = [10, 20, 30]\n"
        "  let v = arr[1]\n"
        "  print(v)\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_for_loop(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let mut s = 0\n"
        "  for let mut i = 0; i < 5; i = i + 1 { s = s + i }\n"
        "  print(s)\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

static int test_interpreter_break_continue(void) {
    ASTNode* prog = parse_source(
        "fn main() {\n"
        "  let mut i = 0\n"
        "  while i < 10 {\n"
        "    i = i + 1\n"
        "    if i == 5 { break }\n"
        "  }\n"
        "  print(i)\n"
        "}"
    );
    mu_assert(prog != NULL, "should parse");
    Interpreter* interp = interpreter_new();
    int ret = interpreter_run(interp, prog);
    mu_assert_eq(ret, 0, "run should succeed");
    interpreter_free(interp);
    ast_free(prog);
    return 0;
}

void test_suite_interpreter(void) {
    mu_run_test(test_interpreter_arithmetic);
    mu_run_test(test_interpreter_fn_call);
    mu_run_test(test_interpreter_if_else);
    mu_run_test(test_interpreter_while_loop);
    mu_run_test(test_interpreter_recursion);
    mu_run_test(test_interpreter_nested_scopes);
    mu_run_test(test_interpreter_bool_logic);
    mu_run_test(test_interpreter_string_concat);
    mu_run_test(test_interpreter_array_index);
    mu_run_test(test_interpreter_for_loop);
    mu_run_test(test_interpreter_break_continue);
}
