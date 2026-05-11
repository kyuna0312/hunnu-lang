#ifndef HUNNU_INTERPRETER_H
#define HUNNU_INTERPRETER_H

#include "ast/ast.h"
#include "value.h"
#include "scope.h"

#define MAX_EXTERN_FNS 64
#define MAX_USER_FNS 256

typedef struct {
    char* name;
    char* symbol_name;
    void* handle;
    void* func_ptr;
    int returns_int;
    int param_count;
} ExternFn;

typedef struct {
    char* name;
    char* parent_name;
    char** fields;
    int* is_pub;
    size_t field_count;
} TypeInfo;

typedef struct {
    char* name;
    ASTNode* node;
} UserFn;

typedef struct Interpreter {
    Scope* current_scope;
    Value return_value;
    int has_return;
    int has_break;
    int has_continue;
    int32_t current_line;
    ExternFn extern_fns[MAX_EXTERN_FNS];
    size_t extern_fn_count;
    TypeInfo* types;
    size_t type_count;
    size_t type_capacity;
    UserFn user_fns[MAX_USER_FNS];
    size_t user_fn_count;
    /* Tail Call Optimization */
    char* current_fn_name;
    int tco_pending;
    Value* tco_args;
    size_t tco_arg_count;
} Interpreter;

/* Lifecycle */
Interpreter* interpreter_new(void);
void interpreter_free(Interpreter* interpreter);
int interpreter_run(Interpreter* interpreter, ASTNode* program);

/* State management */
void interpreter_set_return(Interpreter* interp, Value value);
Value interpreter_get_return(Interpreter* interp);
void interpreter_clear_return(Interpreter* interp);
int interpreter_has_break(Interpreter* interp);
int interpreter_has_continue(Interpreter* interp);
void interpreter_clear_break_continue(Interpreter* interp);

/* Type system helpers */
void interpreter_register_type(Interpreter* interp, const char* name, const char* parent_name, char** fields, int* is_pub, size_t field_count);
TypeInfo* interpreter_lookup_type(Interpreter* interp, const char* name);

/* Evaluation */
Value interpreter_evaluate(Interpreter* interp, ASTNode* node);

/* Call */
Value interpreter_call_user_fn(Interpreter* interp, UserFn* ufn, Value* args, size_t arg_count);

/* Execution */
void interpreter_execute_statement(Interpreter* interp, ASTNode* node);
void interpreter_execute_block_scoped(Interpreter* interp, ASTNode* node);
void interpreter_execute_body(Interpreter* interp, ASTNode* body);

#endif
