#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

struct Lexer {
    const char* source;
    size_t length;
    size_t current;
    size_t start;
    int32_t line;
    int32_t column;
};

static const char* keyword_names[] = {
    "let",
    "fn",
    "if",
    "else",
    "true",
    "false",
    "print",
    NULL
};

static TokenType keyword_types[] = {
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_PRINT,
    TOKEN_UNKNOWN
};

Lexer* lexer_new(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->current = 0;
    lexer->start = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

void lexer_free(Lexer* lexer) {
    free(lexer);
}

int lexer_is_at_end(Lexer* lexer) {
    return lexer->current >= lexer->length;
}

char lexer_peek(Lexer* lexer) {
    if (lexer_is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

char lexer_peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->current + 1];
}

void lexer_advance(Lexer* lexer) {
    if (lexer_is_at_end(lexer)) return;
    
    if (lexer->source[lexer->current] == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    lexer->current++;
}

int lexer_match(Lexer* lexer, char expected) {
    if (lexer_is_at_end(lexer)) return 0;
    if (lexer->source[lexer->current] != expected) return 0;
    
    lexer->current++;
    return 1;
}

int lexer_skip_whitespace(Lexer* lexer) {
    size_t count = 0;
    while (!lexer_is_at_end(lexer)) {
        char c = lexer_peek(lexer);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lexer_advance(lexer);
            count++;
        } else {
            break;
        }
    }
    return count;
}

int lexer_skip_comment(Lexer* lexer) {
    if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '/') {
        while (!lexer_is_at_end(lexer) && lexer_peek(lexer) != '\n') {
            lexer_advance(lexer);
        }
        return 1;
    }
    return 0;
}

TokenType lexer_check_keyword(const char* lexeme) {
    for (int i = 0; keyword_names[i] != NULL; i++) {
        if (strcmp(lexeme, keyword_names[i]) == 0) {
            return keyword_types[i];
        }
    }
    return TOKEN_IDENT;
}

Token* lexer_read_identifier(Lexer* lexer) {
    size_t start_pos = lexer->current - 1;
    int32_t start_line = lexer->line;
    int32_t start_column = lexer->column - 1;
    
    while (isalnum(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
        lexer_advance(lexer);
    }
    
    size_t len = lexer->current - start_pos;
    char* lexeme = (char*)malloc(len + 1);
    strncpy(lexeme, lexer->source + start_pos, len);
    lexeme[len] = '\0';
    
    TokenType type = lexer_check_keyword(lexeme);
    Token* token = token_new(type, lexeme, start_line, start_column);
    
    free(lexeme);
    return token;
}

Token* lexer_read_number(Lexer* lexer) {
    size_t start_pos = lexer->current - 1;
    int32_t start_line = lexer->line;
    int32_t start_column = lexer->column - 1;
    
    while (isdigit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
    
    size_t len = lexer->current - start_pos;
    char* lexeme = (char*)malloc(len + 1);
    strncpy(lexeme, lexer->source + start_pos, len);
    lexeme[len] = '\0';
    
    Token* token = token_new(TOKEN_INT_LITERAL, lexeme, start_line, start_column);
    token->value.int_value = atoi(lexeme);
    
    free(lexeme);
    return token;
}

Token* lexer_read_string(Lexer* lexer) {
    size_t start_pos = lexer->current;
    int32_t start_line = lexer->line;
    int32_t start_column = lexer->column;
    
    char current = lexer_peek(lexer);
    if (current != '"') {
        return token_new(TOKEN_UNKNOWN, "", start_line, start_column);
    }
    
    lexer_advance(lexer);
    
    size_t content_start = lexer->current;
    
    while (!lexer_is_at_end(lexer) && lexer_peek(lexer) != '"') {
        if (lexer_peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        lexer_advance(lexer);
    }
    
    if (lexer_is_at_end(lexer)) {
        return token_new(TOKEN_UNKNOWN, "", start_line, start_column);
    }
    
    size_t end_pos = lexer->current;
    size_t len = end_pos - content_start;
    
    char* value = (char*)malloc(len + 1);
    strncpy(value, lexer->source + content_start, len);
    value[len] = '\0';
    
    Token* token = token_new(TOKEN_STRING_LITERAL, value, start_line, start_column);
    token->value.string_value = value;
    
    lexer_advance(lexer);
    
    return token;
}

Token* lexer_next_token(Lexer* lexer) {
    while (1) {
        while (!lexer_is_at_end(lexer)) {
            char c = lexer_peek(lexer);
            if (c == ' ' || c == '\t' || c == '\r') {
                lexer_advance(lexer);
            } else if (c == '\n') {
                lexer_advance(lexer);
            } else {
                break;
            }
        }
        
        if (lexer_is_at_end(lexer)) {
            return token_new(TOKEN_EOF, "", lexer->line, lexer->column);
        }
        
        if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '/') {
            while (!lexer_is_at_end(lexer) && lexer_peek(lexer) != '\n') {
                lexer_advance(lexer);
            }
            continue;
        }
        
        break;
    }
    
    lexer->start = lexer->current;
    
    if (lexer_is_at_end(lexer)) {
        return token_new(TOKEN_EOF, "", lexer->line, lexer->column);
    }
    
    char c = lexer_peek(lexer);
    
    if (isalpha(c) || c == '_') {
        lexer_advance(lexer);
        return lexer_read_identifier(lexer);
    }
    
    if (isdigit(c)) {
        lexer_advance(lexer);
        return lexer_read_number(lexer);
    }
    
    switch (c) {
        case '(': lexer_advance(lexer); return token_new(TOKEN_LPAREN, "(" , lexer->line, lexer->column);
        case ')': lexer_advance(lexer); return token_new(TOKEN_RPAREN, ")", lexer->line, lexer->column);
        case '{': lexer_advance(lexer); return token_new(TOKEN_LBRACE, "{", lexer->line, lexer->column);
        case '}': lexer_advance(lexer); return token_new(TOKEN_RBRACE, "}", lexer->line, lexer->column);
        case '[': lexer_advance(lexer); return token_new(TOKEN_LBRACKET, "[", lexer->line, lexer->column);
        case ']': lexer_advance(lexer); return token_new(TOKEN_RBRACKET, "]", lexer->line, lexer->column);
        case ';': lexer_advance(lexer); return token_new(TOKEN_SEMICOLON, ";", lexer->line, lexer->column);
        case ',': lexer_advance(lexer); return token_new(TOKEN_COMMA, ",", lexer->line, lexer->column);
        case ':': lexer_advance(lexer); return token_new(TOKEN_COLON, ":", lexer->line, lexer->column);
        case '+': lexer_advance(lexer); return token_new(TOKEN_PLUS, "+", lexer->line, lexer->column);
        case '-': 
            lexer_advance(lexer);
            if (lexer_match(lexer, '>')) {
                return token_new(TOKEN_ARROW, "->", lexer->line, lexer->column);
            }
            return token_new(TOKEN_MINUS, "-", lexer->line, lexer->column);
        case '*': lexer_advance(lexer); return token_new(TOKEN_STAR, "*", lexer->line, lexer->column);
        case '/': lexer_advance(lexer); return token_new(TOKEN_SLASH, "/", lexer->line, lexer->column);
        case '%': lexer_advance(lexer); return token_new(TOKEN_PERCENT, "%", lexer->line, lexer->column);
        case '=': 
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_EQ, "==", lexer->line, lexer->column);
            }
            return token_new(TOKEN_ASSIGN, "=", lexer->line, lexer->column);
        case '!': 
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_NEQ, "!=", lexer->line, lexer->column);
            }
            return token_new(TOKEN_NOT, "!", lexer->line, lexer->column);
        case '<': 
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_LE, "<=", lexer->line, lexer->column);
            }
            return token_new(TOKEN_LT, "<", lexer->line, lexer->column);
        case '>': 
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_GE, ">=", lexer->line, lexer->column);
            }
            return token_new(TOKEN_GT, ">", lexer->line, lexer->column);
        case '"': return lexer_read_string(lexer);
        case '\n': lexer_advance(lexer); return token_new(TOKEN_NEWLINE, "\\n", lexer->line, lexer->column);
    }
    
    char unknown[2] = {c, '\0'};
    return token_new(TOKEN_UNKNOWN, unknown, lexer->line, lexer->column);
}