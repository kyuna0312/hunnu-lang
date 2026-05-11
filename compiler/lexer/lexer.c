/**
 * @file lexer.c
 * @brief Lexical analyzer implementation for Hunnu
 */

#include "lexer.h"
#include "../i18n/i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/** Lexer state */
struct Lexer {
    const char* source;    /**< Source code string */
    size_t length;          /**< Source length */
    size_t current;         /**< Current position */
    size_t start;          /**< Token start position */
    int32_t line;          /**< Current line */
    int32_t column;        /**< Current column */
};

/** English keywords */
static const char* keyword_names[] = {
    "let",        "хувьсагч",   // variable declaration
    "fn",         "функц",    // function
    "if",         "хэрвээ",   // if
    "else",       "эсвэл",    // else  
    "true",       "үнэн",     // true
    "false",      "худал",     // false
    "print",      "хэвлэх",   // print
    "while",      "давталт",  // while
    "for",        "тооллого", // for
    "return",     "буцаах",   // return
    "break",      "зогсоох",  // break
    "continue",   "үргэлжлүүлэх", // continue
    "match",      "тохирох",  // match
    "null",       "хоосон",  // null
    "import",     "импорт",  // import
    "extern",     "гаднах",  // extern
    "try",        "турших",   // try
    "catch",      "барих",    // catch
    "finally",    "эцэст",    // finally
    "type",       "төрөл",    // type (struct)
    "class",      "класс",    // class declaration
    "new",        "шинэ",     // constructor/instantiation
    "pub",        "нийт",     // public field
    "self",       "өөрөө",    // self reference
    "trait",      "ers",     // trait declaration
    "impl",       "хэрэгжүүлэх", // implementation
    "unsafe",     "аюулгүйбус", // unsafe block
    "enum",       "тоолол",  // enum declaration
    NULL
};

static TokenType keyword_types[] = {
    TOKEN_LET,    TOKEN_LET,
    TOKEN_FN,     TOKEN_FN,
    TOKEN_IF,    TOKEN_IF,
    TOKEN_ELSE,   TOKEN_ELSE,
    TOKEN_TRUE,  TOKEN_TRUE,
    TOKEN_FALSE, TOKEN_FALSE,
    TOKEN_PRINT,  TOKEN_PRINT,
    TOKEN_WHILE,  TOKEN_WHILE,
    TOKEN_FOR,    TOKEN_FOR,
    TOKEN_RETURN, TOKEN_RETURN,
    TOKEN_BREAK,  TOKEN_BREAK,
    TOKEN_CONTINUE, TOKEN_CONTINUE,
    TOKEN_MATCH,  TOKEN_MATCH,
    TOKEN_NULL,   TOKEN_NULL,
    TOKEN_IMPORT, TOKEN_IMPORT,
    TOKEN_EXTERN, TOKEN_EXTERN,
    TOKEN_TRY,    TOKEN_TRY,
    TOKEN_CATCH,  TOKEN_CATCH,
    TOKEN_FINALLY, TOKEN_FINALLY,
    TOKEN_TYPE,   TOKEN_TYPE,
    TOKEN_CLASS,  TOKEN_CLASS,
    TOKEN_NEW,    TOKEN_NEW,
    TOKEN_PUB,    TOKEN_PUB,
    TOKEN_SELF,   TOKEN_SELF,
    TOKEN_TRAIT,  TOKEN_TRAIT,
    TOKEN_IMPL,   TOKEN_IMPL,
    TOKEN_UNSAFE, TOKEN_UNSAFE,
    TOKEN_ENUM,   TOKEN_ENUM,
    TOKEN_UNKNOWN
};

/**
 * @brief Creates a new lexer
 * @param source Source code string
 * @return Newly allocated lexer
 */
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

/**
 * @brief Frees lexer memory
 * @param lexer Lexer to free
 */
void lexer_free(Lexer* lexer) {
    free(lexer);
}

/**
 * @brief Checks if at end of source
 * @param lexer Lexer instance
 * @return 1 if at end, 0 otherwise
 */
int lexer_is_at_end(Lexer* lexer) {
    return lexer->current >= lexer->length;
}

/**
 * @brief Peeks at current character without advancing
 * @param lexer Lexer instance
 * @return Current character or '\0'
 */
char lexer_peek(Lexer* lexer) {
    if (lexer_is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

/**
 * @brief Peeks at next character without advancing
 * @param lexer Lexer instance
 * @return Next character or '\0'
 */
char lexer_peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->current + 1];
}

/**
 * @brief Advances to next character
 * @param lexer Lexer instance
 */
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

/**
 * @brief Matches and consumes expected character
 * @param lexer Lexer instance
 * @param expected Expected character
 * @return 1 if matched, 0 otherwise
 */
int lexer_match(Lexer* lexer, char expected) {
    if (lexer_is_at_end(lexer)) return 0;
    if (lexer->source[lexer->current] != expected) return 0;
    
    lexer->current++;
    return 1;
}

/**
 * @brief Skips whitespace characters
 * @param lexer Lexer instance
 * @return Number of whitespace chars skipped
 */
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

/**
 * @brief Skips single-line comments (// ...)
 * @param lexer Lexer instance
 * @return 1 if comment skipped, 0 otherwise
 */
int lexer_skip_comment(Lexer* lexer) {
    if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '/') {
        while (!lexer_is_at_end(lexer) && lexer_peek(lexer) != '\n') {
            lexer_advance(lexer);
        }
        return 1;
    }
    return 0;
}

/**
 * @brief Checks if identifier is a keyword
 * @param lexeme Identifier string
 * @return Token type (keyword or TOKEN_IDENT)
 */
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
    
    while (isalnum(lexer_peek(lexer)) || lexer_peek(lexer) == '_' || ((unsigned char)lexer_peek(lexer) > 127)) {
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
    
    int is_float = 0;
    if (lexer_peek(lexer) == '.') {
        is_float = 1;
        lexer_advance(lexer);
        while (isdigit(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
    }
    
    size_t len = lexer->current - start_pos;
    char* lexeme = (char*)malloc(len + 1);
    strncpy(lexeme, lexer->source + start_pos, len);
    lexeme[len] = '\0';
    
    if (is_float) {
        Token* token = token_new(TOKEN_FLOAT_LITERAL, lexeme, start_line, start_column);
        token->value.float_value = atof(lexeme);
        free(lexeme);
        return token;
    } else {
        Token* token = token_new(TOKEN_INT_LITERAL, lexeme, start_line, start_column);
        token->value.int_value = atoi(lexeme);
        free(lexeme);
        return token;
    }
}

Token* lexer_read_string(Lexer* lexer) {
    int32_t start_line = lexer->line;
    int32_t start_column = lexer->column;
    
    char current = lexer_peek(lexer);
    if (current != '"') {
        return token_new(TOKEN_UNKNOWN, "", start_line, start_column);
    }
    
    lexer_advance(lexer);
    
    size_t capacity = 64;
    size_t length = 0;
    char* value = (char*)malloc(capacity);
    
    while (!lexer_is_at_end(lexer) && lexer_peek(lexer) != '"') {
        char c = lexer_peek(lexer);
        
        if (c == '\\') {
            lexer_advance(lexer);
            char next = lexer_peek(lexer);
            if (next == 'n') {
                if (length + 1 >= capacity) {
                    capacity *= 2;
                    value = (char*)realloc(value, capacity);
                }
                value[length++] = '\n';
                lexer_advance(lexer);
            } else if (next == 't') {
                if (length + 1 >= capacity) {
                    capacity *= 2;
                    value = (char*)realloc(value, capacity);
                }
                value[length++] = '\t';
                lexer_advance(lexer);
            } else if (next == '\\') {
                if (length + 1 >= capacity) {
                    capacity *= 2;
                    value = (char*)realloc(value, capacity);
                }
                value[length++] = '\\';
                lexer_advance(lexer);
            } else if (next == '"') {
                if (length + 1 >= capacity) {
                    capacity *= 2;
                    value = (char*)realloc(value, capacity);
                }
                value[length++] = '"';
                lexer_advance(lexer);
            } else {
                if (length + 1 >= capacity) {
                    capacity *= 2;
                    value = (char*)realloc(value, capacity);
                }
                value[length++] = '\\';
            }
        } else {
            if (length + 1 >= capacity) {
                capacity *= 2;
                value = (char*)realloc(value, capacity);
            }
            value[length++] = c;
            lexer_advance(lexer);
        }
    }
    
    if (lexer_is_at_end(lexer)) {
        free(value);
        return token_new(TOKEN_UNKNOWN, "", start_line, start_column);
    }
    
    value[length] = '\0';
    
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
    
    if (isalpha(c) || c == '_' || ((unsigned char)c > 127)) {
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
        case '+':
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_PLUS_ASSIGN, "+=", lexer->line, lexer->column);
            }
            return token_new(TOKEN_PLUS, "+", lexer->line, lexer->column);
        case '-':
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_MINUS_ASSIGN, "-=", lexer->line, lexer->column);
            }
            if (lexer_match(lexer, '>')) {
                return token_new(TOKEN_ARROW, "->", lexer->line, lexer->column);
            }
            return token_new(TOKEN_MINUS, "-", lexer->line, lexer->column);
        case '*':
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_STAR_ASSIGN, "*=", lexer->line, lexer->column);
            }
            /* Check if this is a dereference (prefix) or multiplication (infix) */
            /* For now, always return TOKEN_STAR and let parser decide */
            return token_new(TOKEN_STAR, "*", lexer->line, lexer->column);
        case '/':
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_SLASH_ASSIGN, "/=", lexer->line, lexer->column);
            }
            return token_new(TOKEN_SLASH, "/", lexer->line, lexer->column);
        case '%': lexer_advance(lexer); return token_new(TOKEN_PERCENT, "%", lexer->line, lexer->column);
        case '=': 
            lexer_advance(lexer);
            if (lexer_match(lexer, '=')) {
                return token_new(TOKEN_EQ, "==", lexer->line, lexer->column);
            }
            if (lexer_match(lexer, '>')) {
                return token_new(TOKEN_FAT_ARROW, "=>", lexer->line, lexer->column);
            }
            return token_new(TOKEN_ASSIGN, "=", lexer->line, lexer->column);

        case '.':
            lexer_advance(lexer);
            return token_new(TOKEN_DOT, ".", lexer->line, lexer->column);

        case '&':
            lexer_advance(lexer);
            return token_new(TOKEN_AMPERSAND, "&", lexer->line, lexer->column);
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
    fprintf(stderr, "[%d:%d] Warning: ", lexer->line, lexer->column);
    i18n_error(ERR_UNKNOWN_CHARACTER, unknown);
    fprintf(stderr, "\n");
    return token_new(TOKEN_UNKNOWN, unknown, lexer->line, lexer->column);
}

int lexer_peek_struct_field(Lexer* lexer) {
    size_t pos = lexer->current;
    while (pos < lexer->length && (lexer->source[pos] == ' ' || lexer->source[pos] == '\t'))
        pos++;
    if (pos >= lexer->length) return 0;
    /* skip optional ( opening parenthesis */
    if (lexer->source[pos] == '(') pos++;
    while (pos < lexer->length && (lexer->source[pos] == ' ' || lexer->source[pos] == '\t'))
        pos++;
    if (pos >= lexer->length) return 0;
    if (!isalpha(lexer->source[pos]) && lexer->source[pos] != '_') return 0;
    pos++;
    while (pos < lexer->length && (isalnum(lexer->source[pos]) || lexer->source[pos] == '_'))
        pos++;
    while (pos < lexer->length && (lexer->source[pos] == ' ' || lexer->source[pos] == '\t'))
        pos++;
    return (pos < lexer->length && lexer->source[pos] == ':');
}