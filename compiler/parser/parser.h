/**
 * @file parser.h
 * @brief Parser declarations for Hunnu
 */

#ifndef HUNNU_PARSER_H
#define HUNNU_PARSER_H

#include "lexer/lexer.h"
#include "ast/ast.h"

/** Parser opaque structure */
typedef struct Parser Parser;

/**
 * @brief Creates a new parser
 * @param lexer Initialized lexer
 * @return Newly allocated parser
 */
Parser* parser_new(Lexer* lexer);

/**
 * @brief Frees parser memory
 * @param parser Parser to free
 */
void parser_free(Parser* parser);

/**
 * @brief Parses source code into AST
 * @param lexer Initialized lexer
 * @return Program AST node
 */
ASTNode* parse(Lexer* lexer);

/* Parsing functions */
ASTNode* parser_parse_program(Parser* parser);
ASTNode* parser_parse_declaration(Parser* parser);
ASTNode* parser_parse_statement(Parser* parser);
ASTNode* parser_parse_block(Parser* parser);
ASTNode* parser_parse_if_statement(Parser* parser);
ASTNode* parser_parse_print_statement(Parser* parser);
ASTNode* parser_parse_expression_statement(Parser* parser);
ASTNode* parser_parse_expression(Parser* parser);
ASTNode* parser_parse_equality(Parser* parser);
ASTNode* parser_parse_comparison(Parser* parser);
ASTNode* parser_parse_addition(Parser* parser);
ASTNode* parser_parse_multiplication(Parser* parser);
ASTNode* parser_parse_unary(Parser* parser);

#endif