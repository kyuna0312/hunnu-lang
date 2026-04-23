#ifndef HUNNU_LEXER_H
#define HUNNU_LEXER_H

#include <stdint.h>
#include "token.h"

typedef struct Lexer Lexer;

Lexer* lexer_new(const char* source);
void lexer_free(Lexer* lexer);
Token* lexer_next_token(Lexer* lexer);
void lexer_advance(Lexer* lexer);
char lexer_peek(Lexer* lexer);
char lexer_peek_next(Lexer* lexer);
int lexer_is_at_end(Lexer* lexer);

int lexer_match(Lexer* lexer, char expected);
int lexer_skip_whitespace(Lexer* lexer);
int lexer_skip_comment(Lexer* lexer);

Token* lexer_read_identifier(Lexer* lexer);
Token* lexer_read_number(Lexer* lexer);
Token* lexer_read_string(Lexer* lexer);

TokenType lexer_check_keyword(const char* lexeme);

#endif