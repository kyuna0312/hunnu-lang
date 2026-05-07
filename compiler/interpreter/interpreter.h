#ifndef HUNNU_INTERPRETER_H
#define HUNNU_INTERPRETER_H

#include "ast/ast.h"
#include "value.h"
#include "scope.h"

typedef struct Interpreter Interpreter;

Interpreter* interpreter_new(void);
void interpreter_free(Interpreter* interpreter);
int interpreter_run(Interpreter* interpreter, ASTNode* program);

void interpreter_set_return(Interpreter* interp, Value value);
Value interpreter_get_return(Interpreter* interp);
void interpreter_clear_return(Interpreter* interp);

int interpreter_has_break(Interpreter* interp);
int interpreter_has_continue(Interpreter* interp);
void interpreter_clear_break_continue(Interpreter* interp);

#endif