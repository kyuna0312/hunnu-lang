#ifndef HUNNU_TOKEN_H
#define HUNNU_TOKEN_H

#include <stdint.h>

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENT,
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,
    
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_PRINT,
    
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_ASSIGN,
    
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_LE,
    TOKEN_GT,
    TOKEN_GE,
    
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    
    TOKEN_ARROW,
    TOKEN_NEWLINE,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int32_t line;
    int32_t column;
    union {
        int64_t int_value;
        char* string_value;
        int bool_value;
    } value;
} Token;

Token* token_new(TokenType type, const char* lexeme, int32_t line, int32_t column);
void token_free(Token* token);
void token_print(Token* token);

const char* token_type_to_string(TokenType type);

#endif