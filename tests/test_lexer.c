#include "../compiler/lexer/lexer.h"
#include "../compiler/lexer/token.h"
#include "minunit.h"

static int test_lexer_integer(void) {
    Lexer* lexer = lexer_new("42");
    Token* t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_INT_LITERAL, "should be int literal");
    mu_assert_eq(t->value.int_value, 42, "value should be 42");
    token_free(t);
    t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_EOF, "should be EOF");
    token_free(t);
    lexer_free(lexer);
    return 0;
}

static int test_lexer_float(void) {
    Lexer* lexer = lexer_new("3.14");
    Token* t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_FLOAT_LITERAL, "should be float literal");
    mu_assert(t->value.float_value > 3.13 && t->value.float_value < 3.15, "value should be ~3.14");
    token_free(t);
    lexer_free(lexer);
    return 0;
}

static int test_lexer_string(void) {
    Lexer* lexer = lexer_new("\"hello\"");
    Token* t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_STRING_LITERAL, "should be string literal");
    mu_assert_str_eq(t->value.string_value, "hello", "string content");
    token_free(t);
    lexer_free(lexer);
    return 0;
}

static int test_lexer_identifiers(void) {
    Lexer* lexer = lexer_new("foo bar _x");
    Token* t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_IDENT, "first should be ident");
    mu_assert_str_eq(t->lexeme, "foo", "lexeme 'foo'");
    token_free(t);

    t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_IDENT, "second should be ident");
    mu_assert_str_eq(t->lexeme, "bar", "lexeme 'bar'");
    token_free(t);

    t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_IDENT, "third should be ident");
    mu_assert_str_eq(t->lexeme, "_x", "lexeme '_x'");
    token_free(t);

    lexer_free(lexer);
    return 0;
}

static int test_lexer_keywords(void) {
    Lexer* lexer = lexer_new("let fn if else while for return class trait impl pub");
    struct { const char* word; TokenType type; } kws[] = {
        {"let", TOKEN_LET}, {"fn", TOKEN_FN}, {"if", TOKEN_IF},
        {"else", TOKEN_ELSE}, {"while", TOKEN_WHILE}, {"for", TOKEN_FOR},
        {"return", TOKEN_RETURN}, {"class", TOKEN_CLASS}, {"trait", TOKEN_TRAIT},
        {"impl", TOKEN_IMPL}, {"pub", TOKEN_PUB},
    };
    for (size_t i = 0; i < sizeof(kws)/sizeof(kws[0]); i++) {
        Token* t = lexer_next_token(lexer);
        mu_assert_eq(t->type, kws[i].type, kws[i].word);
        token_free(t);
    }
    lexer_free(lexer);
    return 0;
}

static int test_lexer_operators(void) {
    Lexer* lexer = lexer_new("+ - * / % == != < > <= >= =");
    struct { const char* op; TokenType type; } ops[] = {
        {"+", TOKEN_PLUS}, {"-", TOKEN_MINUS}, {"*", TOKEN_STAR},
        {"/", TOKEN_SLASH}, {"%", TOKEN_PERCENT}, {"==", TOKEN_EQ},
        {"!=", TOKEN_NEQ}, {"<", TOKEN_LT}, {">", TOKEN_GT},
        {"<=", TOKEN_LE}, {">=", TOKEN_GE}, {"=", TOKEN_ASSIGN},
    };
    for (size_t i = 0; i < sizeof(ops)/sizeof(ops[0]); i++) {
        Token* t = lexer_next_token(lexer);
        mu_assert_eq(t->type, ops[i].type, ops[i].op);
        token_free(t);
    }
    lexer_free(lexer);
    return 0;
}

static int test_lexer_delimiters(void) {
    Lexer* lexer = lexer_new("( ) { } [ ] , . : ->");
    struct { const char* d; TokenType type; } dels[] = {
        {"(", TOKEN_LPAREN}, {")", TOKEN_RPAREN},
        {"{", TOKEN_LBRACE}, {"}", TOKEN_RBRACE},
        {"[", TOKEN_LBRACKET}, {"]", TOKEN_RBRACKET},
        {",", TOKEN_COMMA}, {".", TOKEN_DOT},
        {":", TOKEN_COLON}, {"->", TOKEN_ARROW},
    };
    for (size_t i = 0; i < sizeof(dels)/sizeof(dels[0]); i++) {
        Token* t = lexer_next_token(lexer);
        mu_assert_eq(t->type, dels[i].type, dels[i].d);
        token_free(t);
    }
    lexer_free(lexer);
    return 0;
}

static int test_lexer_bool_literals(void) {
    Lexer* lexer = lexer_new("true false");
    Token* t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_TRUE, "true keyword");
    token_free(t);

    t = lexer_next_token(lexer);
    mu_assert_eq(t->type, TOKEN_FALSE, "false keyword");
    token_free(t);

    lexer_free(lexer);
    return 0;
}

static int test_lexer_line_numbers(void) {
    Lexer* lexer = lexer_new("a\nb\nc");
    Token* t = lexer_next_token(lexer);
    mu_assert_eq(t->line, 1, "first token line 1");
    token_free(t);

    t = lexer_next_token(lexer);
    mu_assert_eq(t->line, 2, "second token line 2");
    token_free(t);

    t = lexer_next_token(lexer);
    mu_assert_eq(t->line, 3, "third token line 3");
    token_free(t);

    lexer_free(lexer);
    return 0;
}

static int test_lexer_newlines_skipped(void) {
    Lexer* lexer = lexer_new("1 + 2");
    Token* t;
    for (int i = 0; i < 3; i++) {
        t = lexer_next_token(lexer);
        mu_assert(t->type != TOKEN_NEWLINE, "no newline tokens");
        token_free(t);
    }
    lexer_free(lexer);
    return 0;
}

void test_suite_lexer(void) {
    mu_run_test(test_lexer_integer);
    mu_run_test(test_lexer_float);
    mu_run_test(test_lexer_string);
    mu_run_test(test_lexer_identifiers);
    mu_run_test(test_lexer_keywords);
    mu_run_test(test_lexer_operators);
    mu_run_test(test_lexer_delimiters);
    mu_run_test(test_lexer_bool_literals);
    mu_run_test(test_lexer_line_numbers);
    mu_run_test(test_lexer_newlines_skipped);
}
