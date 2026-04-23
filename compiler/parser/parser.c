#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Parser {
    Lexer* lexer;
    Token* current;
    Token* previous;
    int had_error;
};

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

static void parser_error(Parser* parser, const char* message) {
    if (parser->had_error) return;
    parser->had_error = 1;
    fprintf(stderr, "[%d:%d] Error: %s\n", 
           parser->previous ? parser->previous->line : 0,
           parser->previous ? parser->previous->column : 0,
           message);
}

static void parser_advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = lexer_next_token(parser->lexer);
}

static int parser_check(Parser* parser, TokenType type) {
    if (!parser->current) return 0;
    return parser->current->type == type;
}

static int parser_match(Parser* parser, TokenType type) {
    if (!parser_check(parser, type)) return 0;
    parser_advance(parser);
    return 1;
}

static void parser_consume(Parser* parser, TokenType type, const char* message) {
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

ASTNode* parser_parse_declaration(Parser* parser) {
    if (parser_match(parser, TOKEN_LET)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected variable name after 'let'");
            return NULL;
        }
        
        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);
        
        parser_consume(parser, TOKEN_ASSIGN, "Expected '=' after variable name");
        
        ASTNode* value = parser_parse_expression(parser);
        
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
    
    return parser_parse_statement(parser);
}

ASTNode* parser_parse_statement(Parser* parser) {
    if (parser_match(parser, TOKEN_IF)) {
        return parser_parse_if_statement(parser);
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
        else_branch = parser_parse_statement(parser);
    }
    
    return ast_if_stmt_create(condition, then_branch, else_branch,
                          parser->previous->line,
                          parser->previous->column);
}

ASTNode* parser_parse_print_statement(Parser* parser) {
    parser_consume(parser, TOKEN_LPAREN, "Expected '(' after 'print'");
    ASTNode* argument = parser_parse_expression(parser);
    parser_consume(parser, TOKEN_RPAREN, "Expected ')' after argument");
    
    return ast_print_stmt_create(argument,
                             parser->previous->line,
                             parser->previous->column);
}

ASTNode* parser_parse_expression_statement(Parser* parser) {
    ASTNode* expr = parser_parse_expression(parser);
    return ast_expr_stmt_create(expr,
                               parser->previous->line,
                               parser->previous->column);
}

ASTNode* parser_parse_expression(Parser* parser) {
    return parser_parse_equality(parser);
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
    
    return parser_parse_primary(parser);
}

ASTNode* parser_parse_primary(Parser* parser) {
    if (parser_match(parser, TOKEN_INT_LITERAL)) {
        return ast_literal_create_int(parser->previous->value.int_value,
                          parser->previous->line,
                          parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_STRING_LITERAL)) {
        return ast_literal_create_string(parser->previous->value.string_value,
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
    
    if (parser_match(parser, TOKEN_IDENT)) {
        char* name = strdup(parser->previous->lexeme);
        return ast_identifier_create(name,
                                   parser->previous->line,
                                   parser->previous->column);
    }
    
    if (parser_match(parser, TOKEN_LPAREN)) {
        ASTNode* expr = parser_parse_expression(parser);
        parser_consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    parser_error(parser, "Expected expression");
    return NULL;
}