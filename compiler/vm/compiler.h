/**
 * @file compiler.h
 * @brief Bytecode compiler declarations
 */

#ifndef HUNNU_COMPILER_H
#define HUNNU_COMPILER_H

#include <stdint.h>
#include <stddef.h>
#include "opcodes.h"
#include "../value.h"
#include "../ast/ast.h"

/** Bytecode chunk */
typedef struct {
    uint8_t* bytecode;
    size_t capacity;
    size_t count;
} BytecodeChunk;

/** Compiled program */
typedef struct {
    BytecodeChunk code;
    Value* constants;
    size_t constant_capacity;
    size_t constant_count;
} CompiledProgram;

/**
 * @brief Compiles AST to bytecode
 * @param program Program AST
 * @return Compiled program
 */
CompiledProgram* compile_program(ASTNode* program);

/**
 * @brief Frees compiled program
 * @param program Program to free
 */
void compiled_program_free(CompiledProgram* program);

/**
 * @brief Disassembles bytecode chunk
 * @param chunk Bytecode chunk
 */
void disassemble_chunk(BytecodeChunk* chunk);

/**
 * @brief Prints compiled program
 * @param program Compiled program
 */
void compiled_program_print(CompiledProgram* program);

#endif