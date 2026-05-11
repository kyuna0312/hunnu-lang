#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ASTNode* parser_parse_expression(Parser* parser) {
    return parser_parse_assignment(parser);
}

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
        } else if (left->type == AST_FIELD_ACCESS) {
            ASTNode* value = parser_parse_assignment(parser);
            return ast_field_assign_create(left->data.field_access.object,
                                           left->data.field_access.field,
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
        ASTNode* operand = parser_parse_unary(parser);
        return ast_address_of_create(operand,
                                    parser->previous->line,
                                    parser->previous->column);
    }

    if (parser_match(parser, TOKEN_STAR)) {
        ASTNode* operand = parser_parse_unary(parser);
        return ast_dereference_create(operand,
                                      parser->previous->line,
                                      parser->previous->column);
    }

    return parser_parse_postfix(parser);
}

ASTNode* parser_parse_postfix(Parser* parser) {
    ASTNode* expr = parser_parse_primary(parser);

    while (parser_match(parser, TOKEN_DOT)) {
        TokenType dt = parser->current->type;
        if (dt == TOKEN_LPAREN || dt == TOKEN_RPAREN || dt == TOKEN_LBRACE ||
            dt == TOKEN_RBRACE || dt == TOKEN_EOF || dt == TOKEN_UNKNOWN ||
            dt == TOKEN_DOT || dt == TOKEN_COMMA || dt == TOKEN_SEMICOLON) {
            parser_error(parser, "Expected field or method name after '.'");
            return expr;
        }
        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);

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

            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after method arguments");
            expr = ast_method_call_create(expr, name, args, arg_count,
                                           parser->previous->line,
                                           parser->previous->column);
            free(name);
        } else {
            expr = ast_field_access_create(expr, name,
                                            parser->previous->line,
                                            parser->previous->column);
            free(name);
        }
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

    if (parser_match(parser, TOKEN_SELF)) {
        return ast_identifier_create("self",
                          parser->previous->line,
                          parser->previous->column);
    }

    if (parser_match(parser, TOKEN_NEW)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected class name after 'new'");
            return ast_literal_create_int(0, parser->previous->line, parser->previous->column);
        }
        char* cls_name = strdup(parser->current->lexeme);
        parser_advance(parser);

        parser_consume(parser, TOKEN_LPAREN, "Expected '(' after class name in 'new'");
        ASTNode** args = NULL;
        size_t arg_count = 0;
        size_t arg_capacity = 4;
        args = (ASTNode**)malloc(sizeof(ASTNode*) * arg_capacity);

        while (!parser_check(parser, TOKEN_RPAREN)) {
            if (arg_count > 0) {
                parser_consume(parser, TOKEN_COMMA, "Expected ',' between arguments");
            }
            args[arg_count++] = parser_parse_expression(parser);
            if (arg_count >= arg_capacity) {
                arg_capacity *= 2;
                args = (ASTNode**)realloc(args, sizeof(ASTNode*) * arg_capacity);
            }
        }
        parser_consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");

        return ast_new_expr_create(cls_name, args, arg_count,
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

        if (parser_check(parser, TOKEN_LPAREN) && lexer_peek_struct_field(parser->lexer)) {
            parser_advance(parser);
            char** field_names = (char**)malloc(sizeof(char*) * 4);
            ASTNode** field_values = (ASTNode**)malloc(sizeof(ASTNode*) * 4);
            size_t field_count = 0;
            size_t field_cap = 4;

            while (!parser_check(parser, TOKEN_RPAREN) && !parser_check(parser, TOKEN_EOF)) {
                if (field_count > 0) {
                    if (parser_check(parser, TOKEN_COMMA)) {
                        parser_advance(parser);
                    }
                }

                TokenType ft = parser->current->type;
                if (ft == TOKEN_LPAREN || ft == TOKEN_RPAREN || ft == TOKEN_LBRACE ||
                    ft == TOKEN_RBRACE || ft == TOKEN_EOF || ft == TOKEN_UNKNOWN ||
                    ft == TOKEN_COMMA || ft == TOKEN_COLON) {
                    parser_error(parser, "Expected field name");
                    break;
                }
                field_names[field_count] = strdup(parser->current->lexeme);
                parser_advance(parser);

                parser_consume(parser, TOKEN_COLON, "Expected ':' after field name");

                field_values[field_count] = parser_parse_expression(parser);
                field_count++;

                if (field_count >= field_cap) {
                    field_cap *= 2;
                    field_names = (char**)realloc(field_names, sizeof(char*) * field_cap);
                    field_values = (ASTNode**)realloc(field_values, sizeof(ASTNode*) * field_cap);
                }
            }

            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after struct fields");
            ASTNode* result = ast_struct_instance_create(name, field_names, field_values,
                                                         field_count,
                                                         parser->previous->line,
                                                         parser->previous->column);
            free(name);
            return result;
        }

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

ASTNode* parser_parse_match_expression(Parser* parser) {
    int32_t line = parser->previous->line;
    int32_t column = parser->previous->column;

    ASTNode* value = parser_parse_expression(parser);

    parser_consume(parser, TOKEN_LBRACE, "Expected '{' after match value");

    ASTNode** patterns = (ASTNode**)malloc(sizeof(ASTNode*) * 8);
    ASTNode** bodies = (ASTNode**)malloc(sizeof(ASTNode*) * 8);
    size_t case_count = 0;
    size_t capacity = 8;

    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        ASTNode* pattern = NULL;

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
            if (strcmp(parser->current->lexeme, "_") == 0) {
                parser_advance(parser);
                pattern = ast_identifier_create("_", parser->previous->line,
                                              parser->previous->column);
            } else {
                char* pname = strdup(parser->current->lexeme);
                parser_advance(parser);
                pattern = ast_identifier_create(pname, parser->previous->line,
                                              parser->previous->column);
                free(pname);
            }
        } else {
            parser_error(parser, "Expected pattern (literal, identifier, or '_')");
            break;
        }

        parser_consume(parser, TOKEN_FAT_ARROW, "Expected '=>' after pattern");

        ASTNode* body = parser_parse_expression(parser);

        if (case_count >= capacity) {
            capacity *= 2;
            patterns = (ASTNode**)realloc(patterns, sizeof(ASTNode*) * capacity);
            bodies = (ASTNode**)realloc(bodies, sizeof(ASTNode*) * capacity);
        }

        patterns[case_count] = pattern;
        bodies[case_count] = body;
        case_count++;

        if (parser_check(parser, TOKEN_COMMA)) {
            parser_advance(parser);
        }
    }

    parser_consume(parser, TOKEN_RBRACE, "Expected '}' after match cases");

    return ast_match_expr_create(value, patterns, bodies, case_count, line, column);
}

ASTNode* parser_parse_interpolated_string(Parser* parser, const char* str_val,
                                         int32_t line, int32_t column) {
    ASTNode* result = NULL;
    const char* p = str_val;
    int32_t current_line = line;
    int32_t current_column = column + 1;

    while (*p) {
        if (*p == '{') {
            p++;
            current_column++;

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
                    result = ast_string_concat_create(result, text_node, current_line, current_column);
                } else {
                    result = text_node;
                }
            }
        }
    }

    return result ? result : ast_literal_create_string("", line, column);
}
