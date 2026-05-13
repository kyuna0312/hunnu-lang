/**
 * @file exec.c
 * @brief Statement execution for the interpreter
 */

#include "interpreter.h"
#include "../i18n/i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

void interpreter_execute_block_scoped(Interpreter* interp, ASTNode* node) {
    if (!node || node->type != AST_BLOCK) return;

    Scope* block_scope = scope_create(16, interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = block_scope;

    for (size_t i = 0; i < node->data.block.count; i++) {
        interpreter_execute_statement(interp, node->data.block.statements[i]);
        if (interp->has_break || interp->has_continue || interp->has_return) {
            break;
        }
    }

    interp->current_scope = old_scope;
    scope_free(block_scope);
}

void interpreter_execute_body(Interpreter* interp, ASTNode* body) {
    if (!body) return;

    if (body->type == AST_BLOCK) {
        for (size_t i = 0; i < body->data.block.count; i++) {
            interpreter_execute_statement(interp, body->data.block.statements[i]);
            if (interp->has_break || interp->has_continue || interp->has_return) {
                break;
            }
        }
    } else {
        interpreter_execute_statement(interp, body);
    }
}

void interpreter_execute_statement(Interpreter* interp, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_VAR_DECL: {
            char* name = node->data.var_decl.name;
            ASTNode* init = node->data.var_decl.initializer;

            Value value = interpreter_evaluate(interp, init);
            scope_define(interp->current_scope, name, value);
            break;
        }

        case AST_BLOCK:
            interpreter_execute_block_scoped(interp, node);
            break;

        case AST_FN_DECL: {
            if (interp->user_fn_count < MAX_USER_FNS) {
                UserFn* ufn = &interp->user_fns[interp->user_fn_count++];
                ufn->name = strdup(node->data.fn_decl.name);
                ufn->node = node;
            } else {
                fprintf(stderr, "Error at line %d: ", node->line);
                fprintf(stderr, "Too many user-defined functions\n");
            }
            break;
        }

        case AST_EXTERN_FN: {
            if (interp->extern_fn_count >= MAX_EXTERN_FNS) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_TOO_MANY_EXTERN_DECLS);
                fprintf(stderr, "\n");
                break;
            }

            ExternFn* ef = &interp->extern_fns[interp->extern_fn_count];
            ef->name = strdup(node->data.extern_fn.name);
            ef->symbol_name = strdup(node->data.extern_fn.symbol_name);
            ef->returns_int = node->data.extern_fn.returns_int;
            ef->param_count = (int)node->data.extern_fn.param_count;
            ef->handle = NULL;
            ef->func_ptr = NULL;

            if (node->data.extern_fn.lib_name) {
                ef->handle = dlopen(node->data.extern_fn.lib_name, RTLD_LAZY);
            } else {
                ef->handle = dlopen(NULL, RTLD_LAZY);
            }

            if (!ef->handle) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_CANNOT_LOAD_LIBRARY, dlerror());
                fprintf(stderr, "\n");
                break;
            }

            ef->func_ptr = dlsym(ef->handle, ef->symbol_name);
            if (!ef->func_ptr) {
                fprintf(stderr, "Error at line %d: ", node->line);
                i18n_error(ERR_CANNOT_FIND_SYMBOL, ef->symbol_name, dlerror());
                fprintf(stderr, "\n");
                dlclose(ef->handle);
                ef->handle = NULL;
                break;
            }

            interp->extern_fn_count++;
            break;
        }

        case AST_IF_STMT: {
            Value condition = interpreter_evaluate(interp, node->data.if_stmt.condition);
            if (value_as_bool(&condition)) {
                interpreter_execute_statement(interp, node->data.if_stmt.then_branch);
            } else if (node->data.if_stmt.else_branch) {
                interpreter_execute_statement(interp, node->data.if_stmt.else_branch);
            }
            value_free(&condition);
            break;
        }

        case AST_PRINT_STMT: {
            Value arg = interpreter_evaluate(interp, node->data.print_stmt.argument);
            value_print(&arg);
            printf("\n");
            value_free(&arg);
            break;
        }

        case AST_EXPR_STMT: {
            Value result = interpreter_evaluate(interp, node->data.expr_stmt.expression);
            value_free(&result);
            break;
        }

        case AST_WHILE_STMT: {
            while (!interp->has_return && !interp->has_break) {
                Value condition = interpreter_evaluate(interp, node->data.while_stmt.condition);
                if (!value_as_bool(&condition)) {
                    value_free(&condition);
                    break;
                }
                value_free(&condition);
                interpreter_execute_body(interp, node->data.while_stmt.body);

                if (interp->has_break) {
                    break;
                }

                if (interp->has_continue) {
                    interp->has_continue = 0;
                }
            }

            if (interp->has_break) {
                interp->has_break = 0;
            }
            if (interp->has_continue) {
                interp->has_continue = 0;
            }
            break;
        }

        case AST_FOR_STMT: {
            if (node->data.for_stmt.initializer) {
                interpreter_execute_statement(interp, node->data.for_stmt.initializer);
            }

            while (!interp->has_return && !interp->has_break) {
                if (node->data.for_stmt.condition) {
                    Value condition = interpreter_evaluate(interp, node->data.for_stmt.condition);
                    if (!value_as_bool(&condition)) {
                        value_free(&condition);
                        break;
                    }
                    value_free(&condition);
                }

                interpreter_execute_body(interp, node->data.for_stmt.body);

                if (interp->has_break) {
                    break;
                }

                if (interp->has_continue) {
                    interp->has_continue = 0;
                }

                if (node->data.for_stmt.update) {
                    Value update_result = interpreter_evaluate(interp, node->data.for_stmt.update);
                    value_free(&update_result);
                }
            }

            if (interp->has_break) {
                interp->has_break = 0;
            }
            if (interp->has_continue) {
                interp->has_continue = 0;
            }
            break;
        }

        case AST_BREAK_STMT: {
            interp->has_break = 1;
            break;
        }

        case AST_CONTINUE_STMT: {
            interp->has_continue = 1;
            break;
        }

        case AST_RETURN_STMT: {
            Value return_value;
            if (node->data.return_stmt.value) {
                return_value = interpreter_evaluate(interp, node->data.return_stmt.value);
            } else {
                return_value = value_create_none();
            }
            interpreter_set_return(interp, return_value);
            break;
        }

        case AST_ASSIGN: {
            Value value = interpreter_evaluate(interp, node->data.assign.value);
            scope_define(interp->current_scope, node->data.assign.name, value);
            break;
        }

        case AST_MATCH_EXPR: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }

        case AST_TRY_STMT: {
            interpreter_execute_statement(interp, node->data.try_stmt.try_block);

            if (node->data.try_stmt.catch_block) {
            }

            if (node->data.try_stmt.finally_block) {
                interpreter_execute_statement(interp, node->data.try_stmt.finally_block);
            }
            break;
        }

        case AST_INDEX_ASSIGN: {
            Value arr = interpreter_evaluate(interp, node->data.index_assign.array);
            Value idx_val = interpreter_evaluate(interp, node->data.index_assign.index);
            Value val = interpreter_evaluate(interp, node->data.index_assign.value);

            if (arr.type == VALUE_ARRAY && idx_val.type == VALUE_INT) {
                int64_t idx = idx_val.value.int_value;
                if (idx >= 0 && (size_t)idx < arr.array_length) {
                    value_free(arr.array_elements[idx]);
                    arr.array_elements[idx] = (Value*)malloc(sizeof(Value));
                    *arr.array_elements[idx] = value_copy(&val);
                } else {
                    fprintf(stderr, "Error at line %d: ", interp->current_line);
                    i18n_error(ERR_INDEX_OUT_OF_BOUNDS, (long)idx, arr.array_length);
                    fprintf(stderr, "\n");
                }
            }

            value_free(&arr);
            value_free(&idx_val);
            value_free(&val);
            break;
        }

        case AST_TYPE_DECL:
        case AST_CLASS_DECL:
        case AST_IMPL_DECL:
        case AST_TRAIT_DECL: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }

        case AST_STRUCT_INSTANCE:
        case AST_METHOD_CALL:
        case AST_FIELD_ACCESS:
        case AST_FIELD_ASSIGN:
        case AST_ADDRESS_OF:
        case AST_DEREFERENCE:
        case AST_NEW_EXPR: {
            Value result = interpreter_evaluate(interp, node);
            value_free(&result);
            break;
        }

        default:
            break;
    }
}
