/**
 * @file parser.c
 * @brief Parser implementation for Hunnu - recursive descent parser
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** Parser state */
struct Parser {
    Lexer* lexer;      /**< Source lexer */
    Token* current;   /**< Current token */
    Token* previous;   /**< Previous token */
    int had_error;     /**< Error flag */
};

/**
 * @brief Creates a new parser
 * @param lexer Initialized lexer
 * @return Newly allocated parser
 */
Parser* parser_new(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current = NULL;
    parser->previous = NULL;
    parser->had_error = 0;
    return parser;
}

/**
 * @brief Frees parser memory
 * @param parser Parser to free
 */
void parser_free(Parser* parser) {
    if (parser->current) token_free(parser->current);
    free(parser);
}

/* Forward declarations for statement parsers */
static ASTNode* parser_parse_while_statement(Parser* parser);
static ASTNode* parser_parse_for_statement(Parser* parser);
static ASTNode* parser_parse_return_statement(Parser* parser);
static ASTNode* parser_parse_break_statement(Parser* parser);
static ASTNode* parser_parse_continue_statement(Parser* parser);
static ASTNode* parser_parse_match_expression(Parser* parser);
static ASTNode* parser_parse_interpolated_string(Parser* parser, const char* str_val, int32_t line, int32_t column);
static ASTNode* parser_parse_try_statement(Parser* parser);

/**
 * @brief Reports parse error
 * @param parser Parser instance
 * @param message Error message
 */
static void parser_error(Parser* parser, const char* message) {
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

/**
 * @brief Advances to next token
 * @param parser Parser instance
 */
static void parser_advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = lexer_next_token(parser->lexer);
}

/**
 * @brief Checks current token type
 * @param parser Parser instance
 * @param type Token type to check
 * @return 1 if match, 0 otherwise
 */
static int parser_check(Parser* parser, TokenType type) {
    if (!parser->current) return 0;
    return parser->current->type == type;
}

/**
 * @brief Skips newline tokens
 * @param parser Parser instance
 */
static void parser_skip_newlines(Parser* parser) {
    while (parser_check(parser, TOKEN_NEWLINE)) {
        parser_advance(parser);
    }
}

/**
 * @brief Matches and consumes token
 * @param parser Parser instance
 * @param type Expected token type
 * @return 1 if matched, 0 otherwise
 */
static int parser_match(Parser* parser, TokenType type) {
    if (!parser_check(parser, type)) return 0;
    parser_advance(parser);
    return 1;
}

static void parser_consume(Parser* parser, TokenType type, const char* message) {
    if (parser_match(parser, type)) return;
    parser_error(parser, message);
}

static ASTNode* parser_parse_assignment(Parser* parser);

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

ASTNode* parser_parse_declaration(Parser* parser) {
    parser_skip_newlines(parser);
    
    if (parser_match(parser, TOKEN_LET)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected variable name after 'let'");
            return NULL;
        }
        
        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);
        
        parser_consume(parser, TOKEN_ASSIGN, "Expected '=' after variable name");
        
        ASTNode* value = parser_parse_expression(parser);
        
        if (parser_check(parser, TOKEN_SEMICOLON)) {
            parser_advance(parser);
        }
        
        return ast_var_decl_create(name, value, 
                              parser->previous->line, 
                              parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_FN)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected function name after 'fn'");
            return NULL;
        }
        
        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);
        
        char** params = NULL;
        size_t param_count = 0;
        
        if (parser_match(parser, TOKEN_LPAREN)) {
            while (!parser_check(parser, TOKEN_RPAREN)) {
                if (param_count > 0) {
                    parser_consume(parser, TOKEN_COMMA, "Expected ',' between parameters");
                }
                
                if (!parser_check(parser, TOKEN_IDENT)) {
                    parser_error(parser, "Expected parameter name");
                    return NULL;
                }
                
                params = (char**)realloc(params, sizeof(char*) * (param_count + 1));
                params[param_count++] = strdup(parser->current->lexeme);
                parser_advance(parser);
            }
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");
        }
        
        if (!parser_check(parser, TOKEN_LBRACE)) {
            parser_error(parser, "Expected function body");
            return NULL;
        }
        
        ASTNode* body = parser_parse_block(parser);
        
        return ast_fn_decl_create(name, params, param_count, body,
                            parser->previous->line,
                            parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_EXTERN)) {
        parser_consume(parser, TOKEN_FN, "Expected 'fn' after 'extern'");
        
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected function name after 'extern fn'");
            return NULL;
        }
        
        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);
        
        char** params = NULL;
        size_t param_count = 0;
        
        if (parser_match(parser, TOKEN_LPAREN)) {
            while (!parser_check(parser, TOKEN_RPAREN)) {
                if (param_count > 0) {
                    parser_consume(parser, TOKEN_COMMA, "Expected ',' between parameters");
                }
                
                if (!parser_check(parser, TOKEN_IDENT)) {
                    parser_error(parser, "Expected parameter name");
                    return NULL;
                }
                
                params = (char**)realloc(params, sizeof(char*) * (param_count + 1));
                params[param_count++] = strdup(parser->current->lexeme);
                parser_advance(parser);
            }
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");
        }
        
        /* Parse return type: -> int | -> float | -> str | -> void */
        int returns_int = 1; /* default: returns int (1) */
        /* returns_int values: 0=void, 1=int, 2=str, 3=float */
        if (parser_match(parser, TOKEN_ARROW)) {
            if (parser_check(parser, TOKEN_IDENT)) {
                if (strcmp(parser->current->lexeme, "int") == 0) {
                    returns_int = 1;
                    parser_advance(parser);
                } else if (strcmp(parser->current->lexeme, "void") == 0) {
                    returns_int = 0;
                    parser_advance(parser);
                } else if (strcmp(parser->current->lexeme, "str") == 0) {
                    returns_int = 2; /* str return */
                    parser_advance(parser);
                } else if (strcmp(parser->current->lexeme, "float") == 0) {
                    returns_int = 3; /* float return */
                    parser_advance(parser);
                } else {
                    parser_error(parser, "Expected 'int', 'float', 'str', or 'void' after '->'");
                    return NULL;
                }
            }
        }
        
        /* Optional: from "library.so" */
        char* lib_name = NULL;
        if (parser_check(parser, TOKEN_IDENT) && strcmp(parser->current->lexeme, "from") == 0) {
            parser_advance(parser);
            if (parser_check(parser, TOKEN_STRING_LITERAL)) {
                lib_name = strdup(parser->current->value.string_value);
                parser_advance(parser);
            } else {
                parser_error(parser, "Expected string literal after 'from'");
                return NULL;
            }
        }
        
        return ast_extern_fn_create(name, lib_name, name, params, param_count, returns_int,
                                    parser->previous->line,
                                    parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_TYPE)) {
        /* Parse: type Point = { x: int, y: int } */
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected type name after 'type'");
            return NULL;
        }
        
        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);
        
        parser_consume(parser, TOKEN_ASSIGN, "Expected '=' after type name");
        parser_consume(parser, TOKEN_LBRACE, "Expected '{' after '='");
        
        char** fields = NULL;
        size_t field_count = 0;
        size_t field_capacity = 4;
        fields = (char**)malloc(sizeof(char*) * field_capacity);
        
        while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
            if (field_count > 0) {
                if (parser_check(parser, TOKEN_COMMA)) {
                    parser_advance(parser);
                }
            }
            
            if (!parser_check(parser, TOKEN_IDENT)) {
                parser_error(parser, "Expected field name");
                break;
            }
            
            fields[field_count++] = strdup(parser->current->lexeme);
            parser_advance(parser);
            
            /* Optional type annotation: x: int */
            if (parser_match(parser, TOKEN_COLON)) {
                /* Skip type annotation for now */
                if (parser_check(parser, TOKEN_IDENT)) {
                    parser_advance(parser);
                }
            }
            
            if (field_count >= field_capacity) {
                field_capacity *= 2;
                fields = (char**)realloc(fields, sizeof(char*) * field_capacity);
            }
        }
        
        parser_consume(parser, TOKEN_RBRACE, "Expected '}' after type fields");
        
        return ast_type_decl_create(name, fields, field_count,
                                    parser->previous->line,
                                    parser->previous->column);
    }
    
    return parser_parse_statement(parser);
}

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
    parser_consume(parser, TOKEN_LPAREN, "Expected '(' after 'while'");
    ASTNode* condition = parser_parse_expression(parser);
    parser_consume(parser, TOKEN_RPAREN, "Expected ')' after condition");
    
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

ASTNode* parser_parse_expression(Parser* parser) {
    return parser_parse_assignment(parser);
}

/* Forward declarations */
static ASTNode* parser_parse_postfix(Parser* parser);
ASTNode* parser_parse_primary(Parser* parser);

ASTNode* parser_parse_assignment(Parser* parser) {
    ASTNode* left = parser_parse_equality(parser);
    
    if (parser_match(parser, TOKEN_ASSIGN)) {
        if (left->type == AST_IDENTIFIER) {
            char* name = strdup(left->data.identifier.name);
            ASTNode* value = parser_parse_assignment(parser);
            return ast_assign_create(name, value, left->line, left->column);
        } else if (left->type == AST_INDEX_EXPR) {
            ASTNode* value = parser_parse_assignment(parser);
            return ast_index_assign_create(left->data.index_expr.array,
                                           left->data.index_expr.index,
                                           value, left->line, left->column);
        }
    }
    
    if (parser_match(parser, TOKEN_PLUS_ASSIGN) ||
        parser_match(parser, TOKEN_MINUS_ASSIGN) ||
        parser_match(parser, TOKEN_STAR_ASSIGN) ||
        parser_match(parser, TOKEN_SLASH_ASSIGN)) {
        TokenType op = parser->previous->type;
        TokenType binop;
        switch (op) {
            case TOKEN_PLUS_ASSIGN: binop = TOKEN_PLUS; break;
            case TOKEN_MINUS_ASSIGN: binop = TOKEN_MINUS; break;
            case TOKEN_STAR_ASSIGN: binop = TOKEN_STAR; break;
            case TOKEN_SLASH_ASSIGN: binop = TOKEN_SLASH; break;
            default: binop = TOKEN_PLUS;
        }
        ASTNode* right = parser_parse_assignment(parser);
        
        if (left->type == AST_IDENTIFIER) {
            char* name = strdup(left->data.identifier.name);
            ASTNode* current = ast_identifier_create(strdup(name), left->line, left->column);
            ASTNode* bin_expr = ast_binary_expr_create(binop, current, right, left->line, left->column);
            return ast_assign_create(name, bin_expr, left->line, left->column);
        } else if (left->type == AST_INDEX_EXPR) {
            ASTNode* arr_node = left->data.index_expr.array;
            ASTNode* idx_node = left->data.index_expr.index;
            ASTNode* current;
            if (arr_node->type == AST_IDENTIFIER) {
                current = ast_index_expr_create(
                    ast_identifier_create(arr_node->data.identifier.name, arr_node->line, arr_node->column),
                    ast_literal_create_int(idx_node->data.literal.value.int_value, idx_node->line, idx_node->column),
                    left->line, left->column);
            } else {
                current = ast_index_expr_create(arr_node, idx_node, left->line, left->column);
            }
            ASTNode* bin_expr = ast_binary_expr_create(binop, current, right, left->line, left->column);
            ASTNode* assign_arr;
            if (arr_node->type == AST_IDENTIFIER) {
                assign_arr = ast_identifier_create(arr_node->data.identifier.name, arr_node->line, arr_node->column);
            } else {
                assign_arr = arr_node;
            }
            ASTNode* assign_idx;
            if (idx_node->type == AST_LITERAL) {
                assign_idx = ast_literal_create_int(idx_node->data.literal.value.int_value, idx_node->line, idx_node->column);
            } else {
                assign_idx = idx_node;
            }
            return ast_index_assign_create(assign_arr, assign_idx, bin_expr, left->line, left->column);
        }
    }
    
    return left;
}

ASTNode* parser_parse_equality(Parser* parser) {
    ASTNode* left = parser_parse_comparison(parser);
    
    while (parser_match(parser, TOKEN_EQ) || parser_match(parser, TOKEN_NEQ)) {
        TokenType op = parser->previous->type;
        ASTNode* right = parser_parse_comparison(parser);
        left = ast_binary_expr_create(op, left, right,
                                parser->previous->line,
                                parser->previous->column);
    }
    
    return left;
}

ASTNode* parser_parse_comparison(Parser* parser) {
    ASTNode* left = parser_parse_addition(parser);
    
    while (parser_match(parser, TOKEN_GT) || parser_match(parser, TOKEN_GE) ||
           parser_match(parser, TOKEN_LT) || parser_match(parser, TOKEN_LE)) {
        TokenType op = parser->previous->type;
        ASTNode* right = parser_parse_addition(parser);
        left = ast_binary_expr_create(op, left, right,
                                parser->previous->line,
                                parser->previous->column);
    }
    
    return left;
}

ASTNode* parser_parse_addition(Parser* parser) {
    ASTNode* left = parser_parse_multiplication(parser);
    
    while (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS)) {
        TokenType op = parser->previous->type;
        ASTNode* right = parser_parse_multiplication(parser);
        left = ast_binary_expr_create(op, left, right,
                                parser->previous->line,
                                parser->previous->column);
    }
    
    return left;
}

ASTNode* parser_parse_multiplication(Parser* parser) {
    ASTNode* left = parser_parse_unary(parser);
    
    while (parser_match(parser, TOKEN_STAR) || parser_match(parser, TOKEN_SLASH) ||
           parser_match(parser, TOKEN_PERCENT)) {
        TokenType op = parser->previous->type;
        ASTNode* right = parser_parse_unary(parser);
        left = ast_binary_expr_create(op, left, right,
                                parser->previous->line,
                                parser->previous->column);
    }
    
    return left;
}

ASTNode* parser_parse_unary(Parser* parser) {
    if (parser_match(parser, TOKEN_MINUS)) {
        TokenType op = parser->previous->type;
        ASTNode* operand = parser_parse_unary(parser);
        return ast_unary_expr_create(op, operand,
                               parser->previous->line,
                               parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_NOT)) {
        TokenType op = parser->previous->type;
        ASTNode* operand = parser_parse_unary(parser);
        return ast_unary_expr_create(op, operand,
                               parser->previous->line,
                               parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_AMPERSAND)) {
        /* Address-of: &expr */
        ASTNode* operand = parser_parse_unary(parser);
        return ast_address_of_create(operand,
                                    parser->previous->line,
                                    parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_STAR)) {
        /* Dereference: *expr (only if not part of multiplication) */
        /* We need to determine if this is prefix (dereference) or infix (multiply) */
        /* For simplicity, assume prefix if previous token was not an expression */
        ASTNode* operand = parser_parse_unary(parser);
        return ast_dereference_create(operand,
                                      parser->previous->line,
                                      parser->previous->column);
    }
    
    return parser_parse_postfix(parser);
}

ASTNode* parser_parse_postfix(Parser* parser) {
    ASTNode* expr = parser_parse_primary(parser);
    
    /* Handle field access: expr.field */
    while (parser_match(parser, TOKEN_DOT)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected field name after '.'");
            return expr;
        }
        char* field = strdup(parser->current->lexeme);
        parser_advance(parser);
        expr = ast_field_access_create(expr, field,
                                        parser->previous->line,
                                        parser->previous->column);
    }
    
    return expr;
}

ASTNode* parser_parse_primary(Parser* parser) {
    if (parser_match(parser, TOKEN_INT_LITERAL)) {
        return ast_literal_create_int(parser->previous->value.int_value,
                          parser->previous->line,
                          parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_FLOAT_LITERAL)) {
        return ast_literal_create_float(parser->previous->value.float_value,
                              parser->previous->line,
                              parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_STRING_LITERAL)) {
        const char* str_val = parser->previous->value.string_value;
        if (strstr(str_val, "{") != NULL) {
            return parser_parse_interpolated_string(parser, str_val,
                                               parser->previous->line,
                                               parser->previous->column);
        }
        return ast_literal_create_string(str_val,
                                       parser->previous->line,
                                       parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_TRUE)) {
        return ast_literal_create_bool(1,
                          parser->previous->line,
                          parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_FALSE)) {
        return ast_literal_create_bool(0,
                          parser->previous->line,
                          parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_NULL) || parser_match(parser, TOKEN_NIL_KEYWORD)) {
        return ast_literal_create_int(0,
                          parser->previous->line,
                          parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_IDENT)) {
        char* name = strdup(parser->previous->lexeme);
        
        if (strcmp(name, "len") == 0 && parser_match(parser, TOKEN_LPAREN)) {
            ASTNode* arg_expr = parser_parse_expression(parser);
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after len argument");
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
            args[0] = arg_expr;
            return ast_call_expr_create("len", args, 1,
                                   parser->previous->line,
                                   parser->previous->column);
        }
        
        if (strcmp(name, "input") == 0 && parser_match(parser, TOKEN_LPAREN)) {
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after input");
            return ast_call_expr_create("input", NULL, 0,
                                   parser->previous->line,
                                   parser->previous->column);
        }
        
        if (strcmp(name, "to_str") == 0 && parser_match(parser, TOKEN_LPAREN)) {
            ASTNode* arg_expr = parser_parse_expression(parser);
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after to_str argument");
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
            args[0] = arg_expr;
            return ast_call_expr_create("to_str", args, 1,
                                   parser->previous->line,
                                   parser->previous->column);
        }
        
        if (strcmp(name, "to_int") == 0 && parser_match(parser, TOKEN_LPAREN)) {
            ASTNode* arg_expr = parser_parse_expression(parser);
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after to_int argument");
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
            args[0] = arg_expr;
            return ast_call_expr_create("to_int", args, 1,
                                   parser->previous->line,
                                   parser->previous->column);
        }
        
        if (strcmp(name, "to_float") == 0 && parser_match(parser, TOKEN_LPAREN)) {
            ASTNode* arg_expr = parser_parse_expression(parser);
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after to_float argument");
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
            args[0] = arg_expr;
            return ast_call_expr_create("to_float", args, 1,
                                   parser->previous->line,
                                   parser->previous->column);
        }
        
        /* Generic function call for any identifier followed by ( */
        if (parser_match(parser, TOKEN_LPAREN)) {
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 4);
            size_t arg_count = 0;
            size_t arg_capacity = 4;
            
            if (!parser_check(parser, TOKEN_RPAREN)) {
                args[arg_count++] = parser_parse_expression(parser);
                while (parser_match(parser, TOKEN_COMMA)) {
                    if (arg_count >= arg_capacity) {
                        arg_capacity *= 2;
                        args = (ASTNode**)realloc(args, sizeof(ASTNode*) * arg_capacity);
                    }
                    args[arg_count++] = parser_parse_expression(parser);
                }
            }
            
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after function arguments");
            return ast_call_expr_create(name, args, arg_count,
                                   parser->previous->line,
                                   parser->previous->column);
        }
        
        ASTNode* identifier = ast_identifier_create(name,
                                                parser->previous->line,
                                                parser->previous->column);
        
        if (parser_match(parser, TOKEN_LBRACKET)) {
            ASTNode* index = parser_parse_expression(parser);
            parser_consume(parser, TOKEN_RBRACKET, "Expected ']' after index");
            return ast_index_expr_create(identifier, index,
                                        parser->previous->line,
                                        parser->previous->column);
        }
        
        return identifier;
    }
    
    /* Handle 'print' as a function call expression */
    if (parser_match(parser, TOKEN_PRINT) || parser_match(parser, TOKEN_PRINT)) {
        parser_consume(parser, TOKEN_LPAREN, "Expected '(' after 'print'");
        ASTNode* argument = parser_parse_expression(parser);
        parser_consume(parser, TOKEN_RPAREN, "Expected ')' after argument");
        
        ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
        args[0] = argument;
        return ast_call_expr_create("print", args, 1,
                               parser->previous->line,
                               parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_LPAREN)) {
        ASTNode* expr = parser_parse_expression(parser);
        parser_consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    if (parser_match(parser, TOKEN_LBRACKET)) {
        ASTNode** elements = (ASTNode**)malloc(sizeof(ASTNode*) * 4);
        size_t count = 0;
        size_t capacity = 4;
        
        if (!parser_check(parser, TOKEN_RBRACKET)) {
            elements[count++] = parser_parse_expression(parser);
            while (parser_match(parser, TOKEN_COMMA)) {
                if (count >= capacity) {
                    capacity *= 2;
                    elements = (ASTNode**)realloc(elements, sizeof(ASTNode*) * capacity);
                }
                elements[count++] = parser_parse_expression(parser);
            }
        }
        
        parser_consume(parser, TOKEN_RBRACKET, "Expected ']' after array elements");
        return ast_array_expr_create(elements, count,
                              parser->previous->line,
                              parser->previous->column);
    }
    
    parser_error(parser, "Expected expression");
    return NULL;
}

/**
 * @brief Parses a match expression
 * @param parser Parser instance
 * @return AST node for match expression
 */
static ASTNode* parser_parse_match_expression(Parser* parser) {
    int32_t line = parser->previous->line;
    int32_t column = parser->previous->column;
    
    /* Parse the value to match against */
    ASTNode* value = parser_parse_expression(parser);
    
    parser_consume(parser, TOKEN_LBRACE, "Expected '{' after match value");
    
    ASTNode** patterns = (ASTNode**)malloc(sizeof(ASTNode*) * 8);
    ASTNode** bodies = (ASTNode**)malloc(sizeof(ASTNode*) * 8);
    size_t case_count = 0;
    size_t capacity = 8;
    
    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        ASTNode* pattern = NULL;
        
        /* Parse pattern - can be literal, identifier, or '_' wildcard */
        if (parser_match(parser, TOKEN_INT_LITERAL)) {
            pattern = ast_literal_create_int(parser->previous->value.int_value,
                                           parser->previous->line,
                                           parser->previous->column);
        } else if (parser_match(parser, TOKEN_FLOAT_LITERAL)) {
            pattern = ast_literal_create_float(parser->previous->value.float_value,
                                             parser->previous->line,
                                             parser->previous->column);
        } else if (parser_match(parser, TOKEN_STRING_LITERAL)) {
            pattern = ast_literal_create_string(parser->previous->value.string_value,
                                              parser->previous->line,
                                              parser->previous->column);
        } else if (parser_match(parser, TOKEN_TRUE)) {
            pattern = ast_literal_create_bool(1, parser->previous->line,
                                            parser->previous->column);
        } else if (parser_match(parser, TOKEN_FALSE)) {
            pattern = ast_literal_create_bool(0, parser->previous->line,
                                            parser->previous->column);
        } else if (parser_check(parser, TOKEN_IDENT)) {
            /* Could be '_' wildcard or an identifier binding */
            if (strcmp(parser->current->lexeme, "_") == 0) {
                parser_advance(parser);
                pattern = ast_identifier_create("_", parser->previous->line,
                                              parser->previous->column);
            } else {
                char* name = strdup(parser->current->lexeme);
                parser_advance(parser);
                pattern = ast_identifier_create(name, parser->previous->line,
                                              parser->previous->column);
                free(name);
            }
        } else {
            parser_error(parser, "Expected pattern (literal, identifier, or '_')");
            break;
        }
        
        parser_consume(parser, TOKEN_FAT_ARROW, "Expected '=>' after pattern");
        
        /* Parse the body expression */
        ASTNode* body = parser_parse_expression(parser);
        
        if (case_count >= capacity) {
            capacity *= 2;
            patterns = (ASTNode**)realloc(patterns, sizeof(ASTNode*) * capacity);
            bodies = (ASTNode**)realloc(bodies, sizeof(ASTNode*) * capacity);
        }
        
        patterns[case_count] = pattern;
        bodies[case_count] = body;
        case_count++;
        
        /* Optional comma between cases */
        if (parser_check(parser, TOKEN_COMMA)) {
            parser_advance(parser);
        }
    }
    
    parser_consume(parser, TOKEN_RBRACE, "Expected '}' after match cases");
    
    return ast_match_expr_create(value, patterns, bodies, case_count, line, column);
}

/**
 * @brief Parses an interpolated string into a concatenation AST
 * @param parser Parser instance
 * @param str_val The string literal value
 * @param line Line number
 * @param column Column number
 * @return AST node representing the concatenated string
 */
static ASTNode* parser_parse_interpolated_string(Parser* parser, const char* str_val,
                                                 int32_t line, int32_t column) {
    ASTNode* result = NULL;
    const char* p = str_val;
    int32_t current_line = line;
    int32_t current_column = column + 1;
    
    while (*p) {
        if (*p == '{') {
            p++;
            current_column++;
            
            /* Parse expression inside braces */
            Lexer* temp_lexer = lexer_new(p);
            lexer_advance(temp_lexer);
            Parser* temp_parser = parser_new(temp_lexer);
            parser_advance(temp_parser);
            
            ASTNode* expr = parser_parse_expression(temp_parser);
            
            if (!expr) {
                lexer_free(temp_lexer);
                parser_free(temp_parser);
                return result ? result : ast_literal_create_string("", line, column);
            }
            
            /* Find where the expression ended */
            Token* last_tok = temp_parser->previous;
            if (last_tok) {
                size_t expr_len = last_tok->lexeme ? strlen(last_tok->lexeme) : 0;
                p += expr_len;
                current_column += (int32_t)expr_len;
            }
            
            if (*p == '}') {
                p++;
                current_column++;
            }
            
            lexer_free(temp_lexer);
            parser_free(temp_parser);
            
            /* Convert expression to string using to_str */
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * 1);
            args[0] = expr;
            ASTNode* to_str_call = ast_call_expr_create("to_str", args, 1,
                                                         current_line, current_column);
            
            if (result) {
                result = ast_string_concat_create(result, to_str_call, current_line, current_column);
            } else {
                result = to_str_call;
            }
        } else {
            /* Collect literal text */
            const char* start = p;
            int32_t text_column = current_column;
            while (*p && *p != '{') {
                if (*p == '\\' && *(p+1)) {
                    p += 2;
                    current_column += 2;
                } else {
                    p++;
                    current_column++;
                }
            }
            
            size_t len = p - start;
            if (len > 0) {
                char* text = (char*)malloc(len + 1);
                strncpy(text, start, len);
                text[len] = '\0';
                
                ASTNode* text_node = ast_literal_create_string(text, current_line, text_column);
                free(text);
                
                if (result) {
                    result = ast_string_concat_create(result, text_node, current_line, text_column);
                } else {
                    result = text_node;
                }
            }
        }
    }
    
    return result ? result : ast_literal_create_string("", line, column);
}

/**
 * @brief Parses a try-catch statement
 * @param parser Parser instance
 * @return AST node for try statement
 */
static ASTNode* parser_parse_try_statement(Parser* parser) {
    int32_t line = parser->previous->line;
    int32_t column = parser->previous->column;
    
    /* Parse try block */
    ASTNode* try_block = parser_parse_statement(parser);
    if (!try_block) {
        return NULL;
    }
    
    /* Optional catch block */
    ASTNode* catch_block = NULL;
    char* catch_var = NULL;
    
    if (parser_match(parser, TOKEN_CATCH)) {
        /* Optional catch variable: catch(e) or catch e */
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
    
    /* Optional finally block */
    ASTNode* finally_block = NULL;
    if (parser_match(parser, TOKEN_FINALLY)) {
        finally_block = parser_parse_statement(parser);
    }
    
    return ast_try_stmt_create(try_block, catch_var, catch_block, finally_block,
                               line, column);
}