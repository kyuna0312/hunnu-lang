#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/ast/ast.h"
#include "minunit.h"

static ASTNode* parse_source(const char* source) {
    Lexer* lexer = lexer_new(source);
    ASTNode* ast = parse(lexer);
    lexer_free(lexer);
    return ast;
}

static int test_parse_literal_int(void) {
    ASTNode* prog = parse_source("print(42)");
    mu_assert(prog != NULL, "should parse");
    mu_assert_eq(prog->type, AST_PROGRAM, "should be program");
    mu_assert_eq(prog->data.program.count, 1, "one statement");
    ASTNode* stmt = prog->data.program.statements[0];
    mu_assert_eq(stmt->type, AST_PRINT_STMT, "should be print stmt");
    ASTNode* arg = stmt->data.print_stmt.argument;
    mu_assert_eq(arg->type, AST_LITERAL, "should be literal");
    mu_assert_eq(arg->data.literal.literal_type, TOKEN_INT_LITERAL, "int literal");
    mu_assert_eq(arg->data.literal.value.int_value, 42, "value 42");
    ast_free(prog);
    return 0;
}

static int test_parse_var_decl(void) {
    ASTNode* prog = parse_source("let x = 10");
    mu_assert(prog != NULL, "should parse");
    mu_assert_eq(prog->data.program.count, 1, "one statement");
    ASTNode* decl = prog->data.program.statements[0];
    mu_assert_eq(decl->type, AST_VAR_DECL, "should be var decl");
    mu_assert_str_eq(decl->data.var_decl.name, "x", "var name");
    ASTNode* init = decl->data.var_decl.initializer;
    mu_assert_eq(init->type, AST_LITERAL, "init should be literal");
    mu_assert_eq(init->data.literal.value.int_value, 10, "init value 10");
    ast_free(prog);
    return 0;
}

static int test_parse_binary_expr(void) {
    ASTNode* prog = parse_source("let x = 3 + 4 * 5");
    mu_assert(prog != NULL, "should parse");
    ASTNode* decl = prog->data.program.statements[0];
    ASTNode* init = decl->data.var_decl.initializer;
    mu_assert_eq(init->type, AST_BINARY_EXPR, "should be binary expr");
    mu_assert_eq(init->data.binary_expr.operator, TOKEN_PLUS, "should be +");
    ASTNode* left = init->data.binary_expr.left;
    mu_assert_eq(left->type, AST_LITERAL, "left should be literal");
    mu_assert_eq(left->data.literal.value.int_value, 3, "left value 3");
    ASTNode* right = init->data.binary_expr.right;
    mu_assert_eq(right->type, AST_BINARY_EXPR, "right should be binary (multiplication)");
    mu_assert_eq(right->data.binary_expr.operator, TOKEN_STAR, "should be *");
    ast_free(prog);
    return 0;
}

static int test_parse_if_stmt(void) {
    ASTNode* prog = parse_source("if x > 0 { print(1) } else { print(2) }");
    mu_assert(prog != NULL, "should parse");
    ASTNode* stmt = prog->data.program.statements[0];
    mu_assert_eq(stmt->type, AST_IF_STMT, "should be if stmt");
    mu_assert(stmt->data.if_stmt.then_branch != NULL, "then branch exists");
    mu_assert(stmt->data.if_stmt.else_branch != NULL, "else branch exists");
    ast_free(prog);
    return 0;
}

static int test_parse_fn_decl(void) {
    ASTNode* prog = parse_source("fn add(a, b) { return a + b }");
    mu_assert(prog != NULL, "should parse");
    ASTNode* decl = prog->data.program.statements[0];
    mu_assert_eq(decl->type, AST_FN_DECL, "should be fn decl");
    mu_assert_str_eq(decl->data.fn_decl.name, "add", "fn name");
    mu_assert_eq(decl->data.fn_decl.param_count, 2, "two params");
    mu_assert_str_eq(decl->data.fn_decl.params[0], "a", "first param");
    mu_assert_str_eq(decl->data.fn_decl.params[1], "b", "second param");
    mu_assert(decl->data.fn_decl.body != NULL, "body exists");
    ast_free(prog);
    return 0;
}

static int test_parse_while_stmt(void) {
    ASTNode* prog = parse_source("while x > 0 { x = x - 1 }");
    mu_assert(prog != NULL, "should parse");
    ASTNode* stmt = prog->data.program.statements[0];
    mu_assert_eq(stmt->type, AST_WHILE_STMT, "should be while stmt");
    mu_assert(stmt->data.while_stmt.condition != NULL, "condition exists");
    mu_assert(stmt->data.while_stmt.body != NULL, "body exists");
    ast_free(prog);
    return 0;
}

static int test_parse_for_stmt(void) {
    ASTNode* prog = parse_source("for let i = 0; i < 10; i = i + 1 { print(i) }");
    mu_assert(prog != NULL, "should parse for loop");
    ASTNode* stmt = prog->data.program.statements[0];
    mu_assert_eq(stmt->type, AST_FOR_STMT, "should be for stmt");
    mu_assert(stmt->data.for_stmt.initializer != NULL, "initializer exists");
    mu_assert(stmt->data.for_stmt.condition != NULL, "condition exists");
    mu_assert(stmt->data.for_stmt.update != NULL, "update exists");
    mu_assert(stmt->data.for_stmt.body != NULL, "body exists");
    ast_free(prog);
    return 0;
}

static int test_parse_match(void) {
    ASTNode* prog = parse_source("fn main() { match x { 1 => 10, _ => 0 } }");
    mu_assert(prog != NULL, "should parse match");
    ASTNode* fn_decl = prog->data.program.statements[0];
    ASTNode* body = fn_decl->data.fn_decl.body;
    ASTNode* match = body->data.block.statements[0];
    mu_assert_eq(match->type, AST_MATCH_EXPR, "should be match expr");
    mu_assert_eq(match->data.match_expr.case_count, 2, "two cases");
    ast_free(prog);
    return 0;
}

static int test_parse_type_decl(void) {
    ASTNode* prog = parse_source("type Point = { x, y }");
    mu_assert(prog != NULL, "should parse type decl");
    ASTNode* decl = prog->data.program.statements[0];
    mu_assert_eq(decl->type, AST_TYPE_DECL, "should be type decl");
    mu_assert_str_eq(decl->data.type_decl.name, "Point", "type name");
    mu_assert_eq(decl->data.type_decl.field_count, 2, "two fields");
    ast_free(prog);
    return 0;
}

static int test_parse_array_expr(void) {
    ASTNode* prog = parse_source("let arr = [1, 2, 3]");
    mu_assert(prog != NULL, "should parse array");
    ASTNode* decl = prog->data.program.statements[0];
    ASTNode* init = decl->data.var_decl.initializer;
    mu_assert_eq(init->type, AST_ARRAY_EXPR, "should be array expr");
    mu_assert_eq(init->data.array_expr.count, 3, "three elements");
    ast_free(prog);
    return 0;
}

static int test_parse_errors(void) {
    Lexer* lexer = lexer_new("let = 1");
    Parser* parser = parser_new(lexer);
    parser_advance(parser);
    ASTNode* ast = parser_parse_program(parser);
    mu_assert(ast == NULL, "should return NULL on error");
    mu_assert(parser->had_error, "parser should report error");
    parser_free(parser);
    lexer_free(lexer);
    return 0;
}

void test_suite_parser(void) {
    mu_run_test(test_parse_literal_int);
    mu_run_test(test_parse_var_decl);
    mu_run_test(test_parse_binary_expr);
    mu_run_test(test_parse_if_stmt);
    mu_run_test(test_parse_fn_decl);
    mu_run_test(test_parse_while_stmt);
    mu_run_test(test_parse_for_stmt);
    mu_run_test(test_parse_match);
    mu_run_test(test_parse_type_decl);
    mu_run_test(test_parse_array_expr);
    mu_run_test(test_parse_errors);
}
