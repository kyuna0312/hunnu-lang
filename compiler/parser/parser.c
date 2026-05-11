#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Parser* parser_new(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current = NULL;
    parser->previous = NULL;
    parser->had_error = 0;
    return parser;
}

void parser_free(Parser* parser) {
    if (parser->current) token_free(parser->current);
    free(parser);
}

void parser_error(Parser* parser, const char* message) {
    if (parser->had_error) return;
    parser->had_error = 1;
    if (parser->previous) {
        fprintf(stderr, "[%d:%d] Error: %s",
               parser->previous->line,
               parser->previous->column,
               message);
        if (parser->previous->type != TOKEN_NEWLINE) {
            fprintf(stderr, " (got '%s')", parser->previous->lexeme);
        }
        fprintf(stderr, "\n");
    } else if (parser->current) {
        fprintf(stderr, "[%d:%d] Error: %s (got '%s')\n",
               parser->current->line,
               parser->current->column,
               message,
               parser->current->lexeme);
    } else {
        fprintf(stderr, "Error: %s\n", message);
    }
}

void parser_advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = lexer_next_token(parser->lexer);
}

int parser_check(Parser* parser, TokenType type) {
    if (!parser->current) return 0;
    return parser->current->type == type;
}

void parser_skip_newlines(Parser* parser) {
    while (parser_check(parser, TOKEN_NEWLINE)) {
        parser_advance(parser);
    }
}

int parser_match(Parser* parser, TokenType type) {
    if (!parser_check(parser, type)) return 0;
    parser_advance(parser);
    return 1;
}

void parser_consume(Parser* parser, TokenType type, const char* message) {
    if (parser_match(parser, type)) return;
    parser_error(parser, message);
}

ASTNode* parse(Lexer* lexer) {
    Parser* parser = parser_new(lexer);
    parser_advance(parser);
    ASTNode* program = parser_parse_program(parser);
    parser_free(parser);
    return program;
}

ASTNode* parser_parse_program(Parser* parser) {
    ASTNode** statements = (ASTNode**)malloc(sizeof(ASTNode*) * 16);
    size_t count = 0;
    size_t capacity = 16;

    while (!parser_check(parser, TOKEN_EOF)) {
        ASTNode* decl = parser_parse_declaration(parser);
        if (decl) {
            if (count >= capacity) {
                capacity *= 2;
                statements = (ASTNode**)realloc(statements, sizeof(ASTNode*) * capacity);
            }
            statements[count++] = decl;
        }
        if (parser->had_error) return NULL;
    }

    return ast_program_create(statements, count);
}
