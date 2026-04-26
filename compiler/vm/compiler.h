#ifndef HUNNU_COMPILER_H
#define HUNNU_COMPILER_H

#include <stdint.h>
#include <stddef.h>
#include "opcodes.h"
#include "../interpreter/interpreter.h"
#include "../ast/ast.h"

typedef struct {
    uint8_t* bytecode;
    size_t capacity;
    size_t count;
} BytecodeChunk;

typedef struct {
    BytecodeChunk code;
    Value* constants;
    size_t constant_capacity;
    size_t constant_count;
} CompiledProgram;

CompiledProgram* compile_program(ASTNode* program);
void compiled_program_free(CompiledProgram* program);
void disassemble_chunk(BytecodeChunk* chunk);
void compiled_program_print(CompiledProgram* program);

#endif