/**
 * @file call.c
 * @brief Function call dispatch for the interpreter
 */

#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Value interpreter_call_user_fn(Interpreter* interp, UserFn* ufn, Value* args, size_t arg_count) {
    if (!ufn || !ufn->node || ufn->node->type != AST_FN_DECL) {
        return value_create_none();
    }

    ASTNode* fn_node = ufn->node;
    ASTNode* body = fn_node->data.fn_decl.body;

    if (arg_count != fn_node->data.fn_decl.param_count) {
        fprintf(stderr, "Error at line %d: ", interp->current_line);
        fprintf(stderr, "Expected %zu arguments but got %zu for function '%s'\n",
                fn_node->data.fn_decl.param_count, arg_count, ufn->name);
        return value_create_none();
    }

    Scope* fn_scope = scope_create(16, interp->current_scope);
    Scope* old_scope = interp->current_scope;
    interp->current_scope = fn_scope;

    for (size_t i = 0; i < arg_count; i++) {
        scope_define(fn_scope, fn_node->data.fn_decl.params[i], args[i]);
    }

    interp->has_return = 0;

    if (body) {
        if (body->type == AST_BLOCK) {
            for (size_t i = 0; i < body->data.block.count; i++) {
                interpreter_execute_statement(interp, body->data.block.statements[i]);
                if (interp->has_return) break;
            }
        } else {
            interpreter_execute_statement(interp, body);
        }
    }

    Value result = value_create_none();
    if (interp->has_return) {
        result = interp->return_value;
        interp->has_return = 0;
    }

    interp->current_scope = old_scope;
    scope_free(fn_scope);

    return result;
}
