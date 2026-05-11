#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

        char* type_name = NULL;
        if (parser_match(parser, TOKEN_DOT)) {
            type_name = name;
            TokenType dt = parser->current->type;
            if (dt == TOKEN_LPAREN || dt == TOKEN_RPAREN || dt == TOKEN_LBRACE ||
                dt == TOKEN_RBRACE || dt == TOKEN_EOF || dt == TOKEN_UNKNOWN ||
                dt == TOKEN_DOT || dt == TOKEN_COMMA || dt == TOKEN_SEMICOLON) {
                parser_error(parser, "Expected method name after 'Type.'");
                free(type_name);
                return NULL;
            }
            name = strdup(parser->current->lexeme);
            parser_advance(parser);
        }

        char** params = NULL;
        size_t param_count = 0;

        if (parser_match(parser, TOKEN_LPAREN)) {
            while (!parser_check(parser, TOKEN_RPAREN)) {
                if (param_count > 0) {
                    parser_consume(parser, TOKEN_COMMA, "Expected ',' between parameters");
                }

                if (!parser_check(parser, TOKEN_IDENT) && !parser_check(parser, TOKEN_SELF)) {
                    parser_error(parser, "Expected parameter name");
                    if (type_name) free(type_name);
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

        ASTNode* fn_node = ast_fn_decl_create(name, params, param_count, body,
                            parser->previous->line,
                            parser->previous->column);

        if (type_name) {
            char* full_name = (char*)malloc(strlen(type_name) + strlen(name) + 2);
            sprintf(full_name, "%s.%s", type_name, name);
            free(fn_node->data.fn_decl.name);
            fn_node->data.fn_decl.name = full_name;
            free(type_name);
        }

        return fn_node;
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

        int returns_int = 1;
        if (parser_match(parser, TOKEN_ARROW)) {
            if (parser_check(parser, TOKEN_IDENT)) {
                if (strcmp(parser->current->lexeme, "int") == 0) {
                    returns_int = 1;
                    parser_advance(parser);
                } else if (strcmp(parser->current->lexeme, "void") == 0) {
                    returns_int = 0;
                    parser_advance(parser);
                } else if (strcmp(parser->current->lexeme, "str") == 0) {
                    returns_int = 2;
                    parser_advance(parser);
                } else if (strcmp(parser->current->lexeme, "float") == 0) {
                    returns_int = 3;
                    parser_advance(parser);
                } else {
                    parser_error(parser, "Expected 'int', 'float', 'str', or 'void' after '->'");
                    return NULL;
                }
            }
        }

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
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected type name after 'type'");
            return NULL;
        }

        char* name = strdup(parser->current->lexeme);
        parser_advance(parser);

        parser_consume(parser, TOKEN_ASSIGN, "Expected '=' after type name");
        parser_consume(parser, TOKEN_LBRACE, "Expected '{' after '='");

        char** fields = NULL;
        int* is_pub = NULL;
        size_t field_count = 0;
        size_t field_capacity = 8;
        fields = (char**)malloc(sizeof(char*) * field_capacity);
        is_pub = (int*)malloc(sizeof(int) * field_capacity);

        while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
            if (field_count > 0) {
                if (parser_check(parser, TOKEN_COMMA)) {
                    parser_advance(parser);
                }
            }

            int pub = parser_match(parser, TOKEN_PUB) ? 1 : 0;

            if (!parser_check(parser, TOKEN_IDENT)) {
                parser_error(parser, "Expected field name");
                break;
            }

            fields[field_count] = strdup(parser->current->lexeme);
            is_pub[field_count] = pub;
            field_count++;
            parser_advance(parser);

            if (parser_match(parser, TOKEN_COLON)) {
                if (parser_check(parser, TOKEN_IDENT)) {
                    parser_advance(parser);
                }
            }

            if (field_count >= field_capacity) {
                field_capacity *= 2;
                fields = (char**)realloc(fields, sizeof(char*) * field_capacity);
                is_pub = (int*)realloc(is_pub, sizeof(int) * field_capacity);
            }
        }

        parser_consume(parser, TOKEN_RBRACE, "Expected '}' after type fields");

        return ast_type_decl_create(name, fields, is_pub, field_count,
                                    parser->previous->line,
                                    parser->previous->column);
    }

    if (parser_match(parser, TOKEN_CLASS)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected class name after 'class'");
            return NULL;
        }

        char* class_name = strdup(parser->current->lexeme);
        parser_advance(parser);

        char* parent_name = NULL;
        if (parser_match(parser, TOKEN_COLON)) {
            if (!parser_check(parser, TOKEN_IDENT)) {
                parser_error(parser, "Expected parent class name after ':'");
                free(class_name);
                return NULL;
            }
            parent_name = strdup(parser->current->lexeme);
            parser_advance(parser);
        }

        parser_consume(parser, TOKEN_LBRACE, "Expected '{' after class name");

        char** fields = NULL;
        int* is_pub = NULL;
        size_t field_count = 0;
        size_t field_capacity = 8;
        fields = (char**)malloc(sizeof(char*) * field_capacity);
        is_pub = (int*)malloc(sizeof(int) * field_capacity);

        ASTNode** methods = NULL;
        size_t method_count = 0;
        size_t method_capacity = 8;
        methods = (ASTNode**)malloc(sizeof(ASTNode*) * method_capacity);

        ASTNode* constructor = NULL;

        while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
            parser_skip_newlines(parser);

            if (parser_check(parser, TOKEN_RBRACE)) break;

            if (parser_check(parser, TOKEN_FN)) {
                parser_advance(parser);

                TokenType cur = parser->current->type;
                if (cur == TOKEN_LPAREN || cur == TOKEN_RPAREN || cur == TOKEN_LBRACE ||
                    cur == TOKEN_RBRACE || cur == TOKEN_EOF || cur == TOKEN_UNKNOWN) {
                    parser_error(parser, "Expected method name after 'fn' in class");
                    break;
                }

                char* method_name = strdup(parser->current->lexeme);
                int is_new = (strcmp(method_name, "new") == 0);
                parser_advance(parser);

                parser_consume(parser, TOKEN_LPAREN, "Expected '(' after method name");

                char** params = NULL;
                size_t param_count = 0;
                size_t param_capacity = 4;
                params = (char**)malloc(sizeof(char*) * param_capacity);

                while (!parser_check(parser, TOKEN_RPAREN)) {
                    if (param_count > 0) {
                        parser_consume(parser, TOKEN_COMMA, "Expected ',' between parameters");
                    }
                    TokenType pt = parser->current->type;
                    if (pt == TOKEN_LPAREN || pt == TOKEN_RPAREN || pt == TOKEN_LBRACE ||
                        pt == TOKEN_RBRACE || pt == TOKEN_EOF || pt == TOKEN_UNKNOWN) {
                        parser_error(parser, "Expected parameter name");
                        break;
                    }
                    params[param_count++] = strdup(parser->current->lexeme);
                    parser_advance(parser);

                    if (param_count >= param_capacity) {
                        param_capacity *= 2;
                        params = (char**)realloc(params, sizeof(char*) * param_capacity);
                    }
                }
                parser_consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");

                if (!parser_check(parser, TOKEN_LBRACE)) {
                    parser_error(parser, "Expected method body");
                    for (size_t i = 0; i < param_count; i++) free(params[i]);
                    free(params);
                    free(method_name);
                    break;
                }
                ASTNode* body = parser_parse_block(parser);

                char* full_name = (char*)malloc(strlen(class_name) + strlen(method_name) + 2);
                sprintf(full_name, "%s.%s", class_name, method_name);
                free(method_name);

                ASTNode* method_node = ast_fn_decl_create(full_name, params, param_count, body,
                                           parser->previous->line,
                                           parser->previous->column);
                free(full_name);

                if (is_new) {
                    constructor = method_node;
                } else {
                    if (method_count >= method_capacity) {
                        method_capacity *= 2;
                        methods = (ASTNode**)realloc(methods, sizeof(ASTNode*) * method_capacity);
                    }
                    methods[method_count++] = method_node;
                }
            } else {
                int pub = parser_match(parser, TOKEN_PUB) ? 1 : 0;

                if (!parser_check(parser, TOKEN_IDENT)) {
                    parser_error(parser, "Expected field name in class");
                    break;
                }

                fields[field_count] = strdup(parser->current->lexeme);
                is_pub[field_count] = pub;
                field_count++;
                parser_advance(parser);

                if (parser_match(parser, TOKEN_COLON)) {
                    if (parser_check(parser, TOKEN_IDENT)) {
                        parser_advance(parser);
                    }
                }

                if (field_count >= field_capacity) {
                    field_capacity *= 2;
                    fields = (char**)realloc(fields, sizeof(char*) * field_capacity);
                    is_pub = (int*)realloc(is_pub, sizeof(int) * field_capacity);
                }
            }
        }

        parser_consume(parser, TOKEN_RBRACE, "Expected '}' after class body");

        ASTNode* result = ast_class_decl_create(class_name, parent_name, fields, is_pub, field_count,
                                       constructor, methods, method_count,
                                       parser->previous->line,
                                       parser->previous->column);
        free(class_name);
        if (parent_name) free(parent_name);
        return result;
    }

    if (parser_match(parser, TOKEN_TRAIT)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected trait name after 'trait'");
            return NULL;
        }
        char* trait_name = strdup(parser->current->lexeme);
        parser_advance(parser);

        parser_consume(parser, TOKEN_LBRACE, "Expected '{' after trait name");

        char** method_names = NULL;
        size_t* method_param_counts = NULL;
        size_t method_count = 0;
        size_t method_capacity = 8;
        method_names = (char**)malloc(sizeof(char*) * method_capacity);
        method_param_counts = (size_t*)malloc(sizeof(size_t) * method_capacity);

        while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
            parser_skip_newlines(parser);
            if (parser_check(parser, TOKEN_RBRACE)) break;

            if (!parser_check(parser, TOKEN_FN)) {
                parser_error(parser, "Expected 'fn' in trait body");
                break;
            }
            parser_advance(parser);

            if (!parser_check(parser, TOKEN_IDENT) && !parser_check(parser, TOKEN_SELF)) {
                parser_error(parser, "Expected method name in trait");
                break;
            }
            method_names[method_count] = strdup(parser->current->lexeme);
            parser_advance(parser);

            parser_consume(parser, TOKEN_LPAREN, "Expected '(' after method name");
            size_t param_count = 0;
            while (!parser_check(parser, TOKEN_RPAREN)) {
                if (param_count > 0)
                    parser_consume(parser, TOKEN_COMMA, "Expected ',' between parameters");
                if (!parser_check(parser, TOKEN_IDENT) && !parser_check(parser, TOKEN_SELF)) {
                    parser_error(parser, "Expected parameter name");
                    break;
                }
                parser_advance(parser);
                param_count++;
            }
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");
            method_param_counts[method_count] = param_count;
            method_count++;

            if (method_count >= method_capacity) {
                method_capacity *= 2;
                method_names = (char**)realloc(method_names, sizeof(char*) * method_capacity);
                method_param_counts = (size_t*)realloc(method_param_counts, sizeof(size_t) * method_capacity);
            }
        }
        parser_consume(parser, TOKEN_RBRACE, "Expected '}' after trait body");

        return ast_trait_decl_create(trait_name, method_names, method_param_counts, method_count,
                                     parser->previous->line, parser->previous->column);
    }

    if (parser_match(parser, TOKEN_IMPL)) {
        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected trait name after 'impl'");
            return NULL;
        }
        char* impl_trait_name = strdup(parser->current->lexeme);
        parser_advance(parser);

        if (!parser_match(parser, TOKEN_FOR)) {
            parser_error(parser, "Expected 'for' after trait name in impl");
            free(impl_trait_name);
            return NULL;
        }

        if (!parser_check(parser, TOKEN_IDENT)) {
            parser_error(parser, "Expected type name after 'for' in impl");
            free(impl_trait_name);
            return NULL;
        }
        char* impl_type_name = strdup(parser->current->lexeme);
        parser_advance(parser);

        parser_consume(parser, TOKEN_LBRACE, "Expected '{' after impl type name");

        ASTNode** impl_methods = NULL;
        size_t impl_method_count = 0;
        size_t impl_method_capacity = 8;
        impl_methods = (ASTNode**)malloc(sizeof(ASTNode*) * impl_method_capacity);

        while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
            parser_skip_newlines(parser);
            if (parser_check(parser, TOKEN_RBRACE)) break;

            if (!parser_check(parser, TOKEN_FN)) {
                parser_error(parser, "Expected 'fn' in impl body");
                break;
            }
            parser_advance(parser);

            if (!parser_check(parser, TOKEN_IDENT) && !parser_check(parser, TOKEN_SELF)) {
                parser_error(parser, "Expected method name in impl");
                break;
            }
            char* impl_method_name = strdup(parser->current->lexeme);
            parser_advance(parser);

            parser_consume(parser, TOKEN_LPAREN, "Expected '(' after method name");
            char** params = NULL;
            size_t param_count = 0;
            size_t param_capacity = 4;
            params = (char**)malloc(sizeof(char*) * param_capacity);

            while (!parser_check(parser, TOKEN_RPAREN)) {
                if (param_count > 0)
                    parser_consume(parser, TOKEN_COMMA, "Expected ',' between parameters");
                TokenType pt = parser->current->type;
                if (pt == TOKEN_LPAREN || pt == TOKEN_RPAREN || pt == TOKEN_LBRACE ||
                    pt == TOKEN_RBRACE || pt == TOKEN_EOF || pt == TOKEN_UNKNOWN) {
                    parser_error(parser, "Expected parameter name");
                    break;
                }
                params[param_count++] = strdup(parser->current->lexeme);
                parser_advance(parser);
                if (param_count >= param_capacity) {
                    param_capacity *= 2;
                    params = (char**)realloc(params, sizeof(char*) * param_capacity);
                }
            }
            parser_consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");

            if (!parser_check(parser, TOKEN_LBRACE)) {
                parser_error(parser, "Expected method body in impl");
                for (size_t i = 0; i < param_count; i++) free(params[i]);
                free(params);
                free(impl_method_name);
                break;
            }
            ASTNode* impl_body = parser_parse_block(parser);

            char* full_impl_name = (char*)malloc(strlen(impl_type_name) + strlen(impl_method_name) + 2);
            sprintf(full_impl_name, "%s.%s", impl_type_name, impl_method_name);
            free(impl_method_name);

            ASTNode* impl_method_node = ast_fn_decl_create(full_impl_name, params, param_count, impl_body,
                                              parser->previous->line,
                                              parser->previous->column);
            free(full_impl_name);

            if (impl_method_count >= impl_method_capacity) {
                impl_method_capacity *= 2;
                impl_methods = (ASTNode**)realloc(impl_methods, sizeof(ASTNode*) * impl_method_capacity);
            }
            impl_methods[impl_method_count++] = impl_method_node;
        }
        parser_consume(parser, TOKEN_RBRACE, "Expected '}' after impl body");

        return ast_impl_decl_create(impl_trait_name, impl_type_name, impl_methods, impl_method_count,
                                    parser->previous->line, parser->previous->column);
    }

    return parser_parse_statement(parser);
}
