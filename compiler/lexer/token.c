/**
 * @file token.c
 * @brief Token type utilities and display
 */

#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char* token_type_names[] = {
    "EOF",
    "IDENT",
    "INT_LITERAL",
    "FLOAT_LITERAL",
    "STRING_LITERAL",
    "BOOL_LITERAL",
    "LET",
    "FN",
    "IF",
    "ELSE",
    "TRUE",
    "FALSE",
    "PRINT",
    "WHILE",
    "FOR",
    "RETURN",
    "BREAK",
    "CONTINUE",
    "MATCH",
    "NULL",
    "NIL_KEYWORD",
    "IMPORT",
    "EXTERN",
    "TRY",
    "CATCH",
    "FINALLY",
    "PUB",
    "SELF",
    "TYPE",
    "CLASS",
    "NEW",
    "DOT",
    "AMPERSAND",
    "TRAIT",
    "IMPL",
    "UNSAFE",
    "ENUM",
    "MUT",
    "PIPE",
    "LPAREN",
    "RPAREN",
    "LBRACE",
    "RBRACE",
    "LBRACKET",
    "RBRACKET",
    "SEMICOLON",
    "COMMA",
    "COLON",
    "ASSIGN",
    "PLUS",
    "MINUS",
    "STAR",
    "SLASH",
    "PERCENT",
    "EQ",
    "NEQ",
    "PLUS_ASSIGN",
    "MINUS_ASSIGN",
    "STAR_ASSIGN",
    "SLASH_ASSIGN",
    "LT",
    "LE",
    "GT",
    "GE",
    "AND",
    "OR",
    "NOT",
    "ARROW",
    "FAT_ARROW",
    "NEWLINE",
    "DEF",
    "END",
    "SYMBOL",
    "DOT_DOT",
    "UNKNOWN"
};

Token* token_new(TokenType type, const char* lexeme, int32_t line, int32_t column) {
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->column = column;
    return token;
}

void token_free(Token* token) {
    if (token) {
        free(token->lexeme);
        if (token->type == TOKEN_STRING_LITERAL || token->type == TOKEN_SYMBOL) {
            free(token->value.string_value);
        }
        free(token);
    }
}

void token_print(Token* token) {
    printf("%s(%d:%d): '%s'", 
           token_type_to_string(token->type), 
           token->line, 
           token->column, 
           token->lexeme);
}

const char* token_type_to_string(TokenType type) {
    if (type >= 0 && type <= TOKEN_UNKNOWN) {
        return token_type_names[type];
    }
    return "UNKNOWN";
}