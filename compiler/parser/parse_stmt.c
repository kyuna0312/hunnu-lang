#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ASTNode* parser_parse_statement(Parser* parser) {
    parser_skip_newlines(parser);

    if (parser_match(parser, TOKEN_IF)) {
        return parser_parse_if_statement(parser);
    }

    if (parser_match(parser, TOKEN_WHILE)) {
        return parser_parse_while_statement(parser);
    }

    if (parser_match(parser, TOKEN_FOR)) {
        return parser_parse_for_statement(parser);
    }

    if (parser_match(parser, TOKEN_RETURN)) {
        return parser_parse_return_statement(parser);
    }

    if (parser_match(parser, TOKEN_BREAK)) {
        return parser_parse_break_statement(parser);
    }

    if (parser_match(parser, TOKEN_CONTINUE)) {
        return parser_parse_continue_statement(parser);
    }

    if (parser_match(parser, TOKEN_MATCH)) {
        return parser_parse_match_expression(parser);
    }

    if (parser_match(parser, TOKEN_TRY)) {
        return parser_parse_try_statement(parser);
    }

    if (parser_match(parser, TOKEN_PRINT)) {
        return parser_parse_print_statement(parser);
    }

    if (parser_check(parser, TOKEN_LBRACE)) {
        return parser_parse_block(parser);
    }

    return parser_parse_expression_statement(parser);
}

ASTNode* parser_parse_block(Parser* parser) {
    parser_consume(parser, TOKEN_LBRACE, "Expected '{'");

    ASTNode** statements = (ASTNode**)malloc(sizeof(ASTNode*) * 16);
    size_t count = 0;
    size_t capacity = 16;

    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        ASTNode* stmt = parser_parse_declaration(parser);
        if (stmt) {
            if (count >= capacity) {
                capacity *= 2;
                statements = (ASTNode**)realloc(statements, sizeof(ASTNode*) * capacity);
            }
            statements[count++] = stmt;
        }
        if (parser->had_error) return NULL;
    }

    parser_consume(parser, TOKEN_RBRACE, "Expected '}' after block");

    return ast_block_create(statements, count,
                       parser->previous ? parser->previous->line : 0,
                       parser->previous ? parser->previous->column : 0);
}

ASTNode* parser_parse_if_statement(Parser* parser) {
    ASTNode* condition = parser_parse_expression(parser);

    ASTNode* then_branch = parser_parse_statement(parser);
    ASTNode* else_branch = NULL;

    if (parser_match(parser, TOKEN_ELSE)) {
        if (parser_match(parser, TOKEN_IF)) {
            else_branch = parser_parse_if_statement(parser);
        } else {
            else_branch = parser_parse_statement(parser);
        }
    }

    return ast_if_stmt_create(condition, then_branch, else_branch,
                          parser->previous->line,
                          parser->previous->column);
}

ASTNode* parser_parse_print_statement(Parser* parser) {
    parser_consume(parser, TOKEN_LPAREN, "Expected '(' after 'print'");
    ASTNode* argument = parser_parse_expression(parser);
    parser_consume(parser, TOKEN_RPAREN, "Expected ')' after argument");

    if (parser_check(parser, TOKEN_SEMICOLON)) {
        parser_advance(parser);
    }

    return ast_print_stmt_create(argument,
                             parser->previous->line,
                             parser->previous->column);
}

ASTNode* parser_parse_while_statement(Parser* parser) {
    int has_parens = parser_match(parser, TOKEN_LPAREN);
    ASTNode* condition = parser_parse_expression(parser);
    if (has_parens) {
        parser_consume(parser, TOKEN_RPAREN, "Expected ')' after condition");
    }

    ASTNode* body = parser_parse_statement(parser);

    return ast_while_stmt_create(condition, body,
                                parser->previous->line,
                                parser->previous->column);
}

ASTNode* parser_parse_for_statement(Parser* parser) {
    int has_parens = parser_match(parser, TOKEN_LPAREN);

    ASTNode* initializer = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        initializer = parser_parse_declaration(parser);
        if (initializer && parser_check(parser, TOKEN_SEMICOLON)) {
            parser_advance(parser);
        }
    } else {
        parser_consume(parser, TOKEN_SEMICOLON, "Expected ';'");
    }

    ASTNode* condition = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        condition = parser_parse_expression(parser);
    }
    parser_consume(parser, TOKEN_SEMICOLON, "Expected ';' after loop condition");

    ASTNode* update = NULL;
    if (!parser_check(parser, has_parens ? TOKEN_RPAREN : TOKEN_LBRACE)) {
        update = parser_parse_expression(parser);
    }

    if (has_parens) {
        parser_consume(parser, TOKEN_RPAREN, "Expected ')' after for clauses");
    }

    ASTNode* body = parser_parse_statement(parser);

    return ast_for_stmt_create(initializer, condition, update, body,
                               parser->previous->line,
                               parser->previous->column);
}

ASTNode* parser_parse_return_statement(Parser* parser) {
    ASTNode* value = NULL;
    if (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        value = parser_parse_expression(parser);
    }

    return ast_return_stmt_create(value,
                                  parser->previous->line,
                                  parser->previous->column);
}

ASTNode* parser_parse_break_statement(Parser* parser) {
    return ast_break_stmt_create(
        parser->previous->line,
        parser->previous->column);
}

ASTNode* parser_parse_continue_statement(Parser* parser) {
    return ast_continue_stmt_create(
        parser->previous->line,
        parser->previous->column);
}

ASTNode* parser_parse_expression_statement(Parser* parser) {
    ASTNode* expr = parser_parse_expression(parser);

    if (parser_check(parser, TOKEN_SEMICOLON)) {
        parser_advance(parser);
    }

    return ast_expr_stmt_create(expr,
                               parser->previous->line,
                               parser->previous->column);
}

ASTNode* parser_parse_try_statement(Parser* parser) {
    int32_t line = parser->previous->line;
    int32_t column = parser->previous->column;

    ASTNode* try_block = parser_parse_statement(parser);
    if (!try_block) {
        return NULL;
    }

    ASTNode* catch_block = NULL;
    char* catch_var = NULL;

    if (parser_match(parser, TOKEN_CATCH)) {
        if (parser_check(parser, TOKEN_LPAREN)) {
            parser_advance(parser);
            if (parser_check(parser, TOKEN_IDENT)) {
                catch_var = strdup(parser->current->lexeme);
                parser_advance(parser);
            }
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after catch variable");
        } else if (parser_check(parser, TOKEN_IDENT)) {
            catch_var = strdup(parser->current->lexeme);
            parser_advance(parser);
        }

        catch_block = parser_parse_statement(parser);
    }

    ASTNode* finally_block = NULL;
    if (parser_match(parser, TOKEN_FINALLY)) {
        finally_block = parser_parse_statement(parser);
    }

    return ast_try_stmt_create(try_block, catch_var, catch_block, finally_block,
                               line, column);
}
