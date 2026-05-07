/**
 * @file token.h
 * @brief Token type definitions and Token struct for the Hunnu lexer
 */

#ifndef HUNNU_TOKEN_H
#define HUNNU_TOKEN_H

#include <stdint.h>

/** Token types recognized by the lexer */
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENT,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,
    
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_PRINT,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_MATCH,
    TOKEN_NULL,
    TOKEN_NIL_KEYWORD,
    TOKEN_IMPORT,
    TOKEN_EXTERN,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_FINALLY,
    TOKEN_TYPE,
    TOKEN_DOT,
    TOKEN_AMPERSAND,
    
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
    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_STAR_ASSIGN,
    TOKEN_SLASH_ASSIGN,
    TOKEN_LT,
    TOKEN_LE,
    TOKEN_GT,
    TOKEN_GE,
    
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    
    TOKEN_ARROW,
    TOKEN_FAT_ARROW,
    TOKEN_NEWLINE,
    TOKEN_UNKNOWN
} TokenType;

/** Token structure representing a lexical token */
typedef struct {
    TokenType type;       /**< Token type */
    char* lexeme;       /**< Original text representation */
    int32_t line;       /**< Line number in source */
    int32_t column;     /**< Column number in source */
    union {
        int64_t int_value;     /**< Integer literal value */
        double float_value;    /**< Float literal value */
        char* string_value;     /**< String literal value */
        int bool_value;        /**< Boolean literal value */
    } value;
} Token;

/**
 * @brief Creates a new token
 * @param type Token type
 * @param lexeme Original text
 * @param line Line number
 * @param column Column number
 * @return Newly allocated token
 */
Token* token_new(TokenType type, const char* lexeme, int32_t line, int32_t column);

/**
 * @brief Frees a token
 * @param token Token to free
 */
void token_free(Token* token);

/**
 * @brief Prints a token to stdout
 * @param token Token to print
 */
void token_print(Token* token);

/**
 * @brief Converts token type to string
 * @param type Token type
 * @return String representation
 */
const char* token_type_to_string(TokenType type);

#endif