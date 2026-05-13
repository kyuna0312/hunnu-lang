/**
 * @file lexer.h
 * @brief Lexer/Tokenizer for Hunnu programming language
 * 
 * The lexer converts source code into a stream of tokens.
 * It handles identifiers, keywords, numbers, strings, and operators.
 */

#ifndef HUNNU_LEXER_H
#define HUNNU_LEXER_H

#include <stdint.h>
#include <stddef.h>
#include "token.h"

/**
 * @brief Opaque lexer structure
 */
typedef struct Lexer Lexer;

/**
 * @brief Create a new lexer from source code
 * @param source The source code string
 * @return Pointer to new lexer instance
 */
Lexer* lexer_new(const char* source);

/**
 * @brief Free lexer resources
 * @param lexer Pointer to lexer to free
 */
void lexer_free(Lexer* lexer);

/**
 * @brief Get next token from source
 * @param lexer Pointer to lexer
 * @return Pointer to next token, or NULL at end of input
 */
Token* lexer_next_token(Lexer* lexer);

/**
 * @brief Advance lexer by one character
 * @param lexer Pointer to lexer
 */
void lexer_advance(Lexer* lexer);

/**
 * @brief Peek at current character without advancing
 * @param lexer Pointer to lexer
 * @return Current character
 */
char lexer_peek(Lexer* lexer);

/**
 * @brief Peek at next character without advancing
 * @param lexer Pointer to lexer
 * @return Next character
 */
char lexer_peek_next(Lexer* lexer);

/**
 * @brief Check if lexer has reached end of input
 * @param lexer Pointer to lexer
 * @return 1 if at end, 0 otherwise
 */
int lexer_is_at_end(Lexer* lexer);

/**
 * @brief Get current position in the source
 * @param lexer Pointer to lexer
 * @return Current offset from start of source
 */
size_t lexer_get_position(Lexer* lexer);

/**
 * @brief Match and consume expected character
 * @param lexer Pointer to lexer
 * @param expected Character to match
 * @return 1 if matched, 0 otherwise
 */
int lexer_match(Lexer* lexer, char expected);

/**
 * @brief Read identifier or keyword
 * @param lexer Pointer to lexer
 * @return Token for identifier or keyword
 */
Token* lexer_read_identifier(Lexer* lexer);

/**
 * @brief Read numeric literal (integer or float)
 * @param lexer Pointer to lexer
 * @return Token for number
 */
Token* lexer_read_number(Lexer* lexer);

/**
 * @brief Read string literal
 * @param lexer Pointer to lexer
 * @return Token for string
 */
Token* lexer_read_string(Lexer* lexer);

/**
 * @brief Check if identifier is a keyword
 * @param lexeme Identifier string to check
 * @return TokenType (keyword type or TOKEN_IDENT)
 */
TokenType lexer_check_keyword(const char* lexeme);

/**
 * @brief Peek at the source to check if upcoming content is IDENT : (struct field)
 * @param lexer Pointer to lexer
 * @return 1 if upcoming non-whitespace chars match IDENT : pattern, 0 otherwise
 */
int lexer_peek_struct_field(Lexer* lexer);

/**
 * @brief Check if current position starts a symbol literal (:ident)
 * @param lexer Pointer to lexer
 * @return 1 if current char is ':' followed by identifier start, 0 otherwise
 */
int lexer_peek_symbol(Lexer* lexer);

#endif