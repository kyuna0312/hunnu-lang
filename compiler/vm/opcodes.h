/**
 * @file opcodes.h
 * @brief VM opcodes and instructions
 */

#ifndef HUNNU_OPCODES_H
#define HUNNU_OPCODES_H

#include <stdint.h>

/** VM opcodes */
typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_INT,
    OP_CONSTANT_FLOAT,
    OP_CONSTANT_STRING,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    
    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
    
    OP_NOT,
    OP_NEGATE,
    
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    
    OP_POP,
    OP_POPN,
    
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    
    OP_CREATE_ARRAY,
    OP_GET_INDEX,
    OP_SET_INDEX,
    
    OP_CREATE_STRING,
    
    OP_CALL,
    OP_RETURN,
    
    OP_DEFINE_FN,
    
    OP_HALT
} OpCode;

/** VM instruction */
typedef struct {
    OpCode code;
    int64_t operand;
} Instruction;

#endif