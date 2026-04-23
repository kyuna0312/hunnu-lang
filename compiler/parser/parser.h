#ifndef HUNNU_PARSER_H
#define HUNNU_PARSER_H

#include "lexer/lexer.h"
#include "ast/ast.h"

typedef struct Parser Parser;

Parser* parser_new(Lexer* lexer);
void parser_free(Parser* parser);
ASTNode* parse(Lexer* lexer);

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
ASTNode* parser_parse_primary(Parser* parser);

#endif