/**
 * @file interpreter.h
 * @brief Interpreter declarations for Hunnu runtime
 */

#ifndef HUNNU_INTERPRETER_H
#define HUNNU_INTERPRETER_H

#include "ast/ast.h"

/** Interpreter opaque structure */
typedef struct Interpreter Interpreter;

/**
 * @brief Creates a new interpreter
 * @return Newly allocated interpreter
 */
Interpreter* interpreter_new(void);

/**
 * @brief Frees interpreter memory
 * @param interpreter Interpreter to free
 */
void interpreter_free(Interpreter* interpreter);

/**
 * @brief Runs the AST program
 * @param interpreter Interpreter instance
 * @param program Program AST
 * @return 0 on success, non-zero on error
 */
int interpreter_run(Interpreter* interpreter, ASTNode* program);

/** Value types */
typedef struct Value {
    enum {
        VALUE_INT,      /**< Integer */
        VALUE_FLOAT,    /**< Float */
        VALUE_STRING,   /**< String */
        VALUE_BOOL,     /**< Boolean */
        VALUE_NONE,     /**< None/null */
        VALUE_ARRAY,    /**< Array */
        VALUE_STRUCT,   /**< Struct/record */
        VALUE_POINTER   /**< Pointer */
    } type;
    union {
        int64_t int_value;
        double float_value;
        char* string_value;
        int bool_value;
        void* pointer_value;  /**< For struct pointer or raw pointer */
    } value;
    int has_value;
    size_t array_length;
    struct Value** array_elements;
    /* For struct types */
    char* struct_type;           /**< Struct type name */
    struct Value** struct_fields;  /**< Struct field values */
    size_t struct_field_count;    /**< Number of fields */
} Value;

/* Return value management */
void interpreter_set_return(Interpreter* interp, Value value);
Value interpreter_get_return(Interpreter* interp);
void interpreter_clear_return(Interpreter* interp);

/* Loop control */
int interpreter_has_break(Interpreter* interp);
int interpreter_has_continue(Interpreter* interp);
void interpreter_clear_break_continue(Interpreter* interp);

/* Value operations */
void value_free(Value* value);
Value value_copy(const Value* value);
void value_print(Value* value);
int value_as_bool(Value* value);
int64_t value_as_int(Value* value);

/* Value creation */
Value value_create_int(int64_t val);
Value value_create_float(double val);
Value value_create_string(char* val);
Value value_create_bool(int val);
Value value_create_none(void);
Value value_create_array(size_t length);
Value value_create_array_val(Value** arr, size_t length);
char* value_to_string(Value* value);

#endif