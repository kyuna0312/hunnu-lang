#include "compiler.h"
#include "../value.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint8_t* bytecode;
    size_t capacity;
    size_t count;
    
    Value* constants;
    size_t constant_capacity;
    size_t constant_count;
    
    int local_count;
} Compiler;

static void compiler_init(Compiler* c) {
    c->capacity = 256;
    c->bytecode = (uint8_t*)malloc(c->capacity);
    c->count = 0;
    
    c->constant_capacity = 64;
    c->constants = (Value*)malloc(sizeof(Value) * c->constant_capacity);
    c->constant_count = 0;
    
    c->local_count = 0;
}

static void compiler_emit_byte(Compiler* c, uint8_t byte) {
    if (c->count >= c->capacity) {
        c->capacity *= 2;
        c->bytecode = (uint8_t*)realloc(c->bytecode, c->capacity);
    }
    c->bytecode[c->count++] = byte;
}

static void compiler_emit_int(Compiler* c, int64_t value) {
    for (int i = 0; i < 8; i++) {
        compiler_emit_byte(c, (value >> (i * 8)) & 0xFF);
    }
}

static size_t compiler_emit_constant(Compiler* c, Value value) {
    if (c->constant_count >= c->constant_capacity) {
        c->constant_capacity *= 2;
        c->constants = (Value*)realloc(c->constants, sizeof(Value) * c->constant_capacity);
    }
    c->constants[c->constant_count] = value;
    return c->constant_count++;
}

static void compiler_compile_expression(Compiler* c, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM: {
            for (size_t i = 0; i < node->data.program.count; i++) {
                compiler_compile_expression(c, node->data.program.statements[i]);
            }
            break;
        }
        
        case AST_FN_DECL: {
            // For now, just compile the function body
            // Function definitions will be handled differently in a full implementation
            compiler_compile_expression(c, node->data.fn_decl.body);
            break;
        }
        
        case AST_BLOCK: {
            for (size_t i = 0; i < node->data.block.count; i++) {
                compiler_compile_expression(c, node->data.block.statements[i]);
            }
            break;
        }
        
        case AST_EXPR_STMT: {
            compiler_compile_expression(c, node->data.expr_stmt.expression);
            break;
        }
        
        case AST_VAR_DECL: {
            c->local_count++;
            if (node->data.var_decl.initializer) {
                compiler_compile_expression(c, node->data.var_decl.initializer);
            } else {
                compiler_emit_byte(c, OP_NIL);
            }
            compiler_emit_byte(c, OP_SET_LOCAL);
            compiler_emit_byte(c, (uint8_t)c->local_count - 1);
            break;
        }
        
        case AST_LITERAL: {
            if (node->data.literal.literal_type == TOKEN_INT_LITERAL) {
                compiler_emit_byte(c, OP_CONSTANT_INT);
                compiler_emit_int(c, node->data.literal.value.int_value);
            } else if (node->data.literal.literal_type == TOKEN_FLOAT_LITERAL) {
                compiler_emit_byte(c, OP_CONSTANT_FLOAT);
                int64_t bits = *(int64_t*)&node->data.literal.value.float_value;
                compiler_emit_int(c, bits);
            } else if (node->data.literal.literal_type == TOKEN_STRING_LITERAL) {
                Value v = value_create_string(node->data.literal.value.string_value);
                size_t idx = compiler_emit_constant(c, v);
                compiler_emit_byte(c, OP_CONSTANT_STRING);
                compiler_emit_byte(c, (uint8_t)idx);
            } else if (node->data.literal.literal_type == TOKEN_BOOL_LITERAL) {
                if (node->data.literal.value.bool_value) {
                    compiler_emit_byte(c, OP_TRUE);
                } else {
                    compiler_emit_byte(c, OP_FALSE);
                }
            }
            break;
        }
        
        case AST_BINARY_EXPR: {
            compiler_compile_expression(c, node->data.binary_expr.left);
            compiler_compile_expression(c, node->data.binary_expr.right);
            
            switch (node->data.binary_expr.operator) {
                case TOKEN_PLUS: compiler_emit_byte(c, OP_ADD); break;
                case TOKEN_MINUS: compiler_emit_byte(c, OP_SUBTRACT); break;
                case TOKEN_STAR: compiler_emit_byte(c, OP_MULTIPLY); break;
                case TOKEN_SLASH: compiler_emit_byte(c, OP_DIVIDE); break;
                case TOKEN_PERCENT: compiler_emit_byte(c, OP_MODULO); break;
                case TOKEN_GT: compiler_emit_byte(c, OP_GREATER); break;
                case TOKEN_LT: compiler_emit_byte(c, OP_LESS); break;
                case TOKEN_GE: compiler_emit_byte(c, OP_GREATER_EQUAL); break;
                case TOKEN_LE: compiler_emit_byte(c, OP_LESS_EQUAL); break;
                case TOKEN_EQ: compiler_emit_byte(c, OP_EQUAL); break;
                case TOKEN_NEQ: compiler_emit_byte(c, OP_NOT_EQUAL); break;
                default: break;
            }
            break;
        }
        
        case AST_IDENTIFIER: {
            compiler_emit_byte(c, OP_GET_LOCAL);
            compiler_emit_byte(c, 0);
            break;
        }
        
        case AST_ASSIGN: {
            compiler_compile_expression(c, node->data.assign.value);
            compiler_emit_byte(c, OP_SET_LOCAL);
            compiler_emit_byte(c, 0);
            break;
        }
        
        case AST_IF_STMT: {
            compiler_compile_expression(c, node->data.if_stmt.condition);
            compiler_emit_byte(c, OP_JUMP_IF_FALSE);
            size_t jump_pos = c->count;
            compiler_emit_int(c, 0);
            
            compiler_compile_expression(c, node->data.if_stmt.then_branch);
            
            if (node->data.if_stmt.else_branch) {
                compiler_emit_byte(c, OP_JUMP);
                size_t else_jump_pos = c->count;
                compiler_emit_int(c, 0);
                
                int64_t offset = c->count - jump_pos - 8;
                for (int i = 0; i < 8; i++) {
                    c->bytecode[jump_pos + i] = (offset >> (i * 8)) & 0xFF;
                }
                
                compiler_compile_expression(c, node->data.if_stmt.else_branch);
                
                offset = c->count - else_jump_pos - 8;
                for (int i = 0; i < 8; i++) {
                    c->bytecode[else_jump_pos + i] = (offset >> (i * 8)) & 0xFF;
                }
            } else {
                int64_t offset = c->count - jump_pos - 8;
                for (int i = 0; i < 8; i++) {
                    c->bytecode[jump_pos + i] = (offset >> (i * 8)) & 0xFF;
                }
            }
            break;
        }
        
        case AST_WHILE_STMT: {
            size_t loop_start = c->count;
            
            compiler_compile_expression(c, node->data.while_stmt.condition);
            compiler_emit_byte(c, OP_JUMP_IF_FALSE);
            size_t exit_jump = c->count;
            compiler_emit_int(c, 0);
            
            compiler_compile_expression(c, node->data.while_stmt.body);
            
            compiler_emit_byte(c, OP_JUMP);
            compiler_emit_int(c, loop_start - c->count - 8);
            
            int64_t offset = c->count - exit_jump - 8;
            for (int i = 0; i < 8; i++) {
                c->bytecode[exit_jump + i] = (offset >> (i * 8)) & 0xFF;
            }
            break;
        }
        
        case AST_PRINT_STMT: {
            compiler_compile_expression(c, node->data.print_stmt.argument);
            compiler_emit_byte(c, OP_CONSTANT_STRING);
            size_t idx = compiler_emit_constant(c, value_create_string("print"));
            compiler_emit_byte(c, (uint8_t)idx);
            compiler_emit_byte(c, OP_CALL);
            compiler_emit_byte(c, 1);
            break;
        }
        
        case AST_RETURN_STMT: {
            if (node->data.return_stmt.value) {
                compiler_compile_expression(c, node->data.return_stmt.value);
            } else {
                compiler_emit_byte(c, OP_NIL);
            }
            compiler_emit_byte(c, OP_RETURN);
            break;
        }
        
        case AST_ARRAY_EXPR: {
            for (size_t i = 0; i < node->data.array_expr.count; i++) {
                compiler_compile_expression(c, node->data.array_expr.elements[i]);
            }
            compiler_emit_byte(c, OP_CREATE_ARRAY);
            compiler_emit_byte(c, (uint8_t)node->data.array_expr.count);
            break;
        }
        
        case AST_INDEX_EXPR: {
            compiler_compile_expression(c, node->data.index_expr.array);
            compiler_compile_expression(c, node->data.index_expr.index);
            compiler_emit_byte(c, OP_GET_INDEX);
            break;
        }
        
        case AST_INDEX_ASSIGN: {
            compiler_compile_expression(c, node->data.index_assign.array);
            compiler_compile_expression(c, node->data.index_assign.index);
            compiler_compile_expression(c, node->data.index_assign.value);
            compiler_emit_byte(c, OP_SET_INDEX);
            break;
        }
        
        case AST_CALL_EXPR: {
            for (size_t i = 0; i < node->data.call_expr.arg_count; i++) {
                compiler_compile_expression(c, node->data.call_expr.args[i]);
            }
            compiler_emit_byte(c, OP_CALL);
            compiler_emit_byte(c, (uint8_t)node->data.call_expr.arg_count);
            break;
        }
        
        default:
            break;
    }
}

CompiledProgram* compile_program(ASTNode* program) {
    Compiler c;
    compiler_init(&c);
    
    compiler_compile_expression(&c, program);
    compiler_emit_byte(&c, OP_HALT);
    
    CompiledProgram* prog = (CompiledProgram*)malloc(sizeof(CompiledProgram));
    prog->code.bytecode = c.bytecode;
    prog->code.capacity = c.capacity;
    prog->code.count = c.count;
    prog->constants = c.constants;
    prog->constant_capacity = c.constant_capacity;
    prog->constant_count = c.constant_count;
    
    return prog;
}

void compiled_program_free(CompiledProgram* program) {
    if (!program) return;
    free(program->code.bytecode);
    free(program->constants);
    free(program);
}

static const char* op_name(OpCode op) {
    switch (op) {
        case OP_CONSTANT: return "CONSTANT";
        case OP_CONSTANT_INT: return "CONSTANT_INT";
        case OP_CONSTANT_FLOAT: return "CONSTANT_FLOAT";
        case OP_CONSTANT_STRING: return "CONSTANT_STRING";
        case OP_NIL: return "NIL";
        case OP_TRUE: return "TRUE";
        case OP_FALSE: return "FALSE";
        case OP_ADD: return "ADD";
        case OP_SUBTRACT: return "SUBTRACT";
        case OP_MULTIPLY: return "MULTIPLY";
        case OP_DIVIDE: return "DIVIDE";
        case OP_MODULO: return "MODULO";
        case OP_GREATER: return "GREATER";
        case OP_LESS: return "LESS";
        case OP_GREATER_EQUAL: return "GREATER_EQUAL";
        case OP_LESS_EQUAL: return "LESS_EQUAL";
        case OP_EQUAL: return "EQUAL";
        case OP_NOT_EQUAL: return "NOT_EQUAL";
        case OP_NOT: return "NOT";
        case OP_NEGATE: return "NEGATE";
        case OP_JUMP: return "JUMP";
        case OP_JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OP_JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case OP_POP: return "POP";
        case OP_POPN: return "POPN";
        case OP_GET_LOCAL: return "GET_LOCAL";
        case OP_SET_LOCAL: return "SET_LOCAL";
        case OP_GET_GLOBAL: return "GET_GLOBAL";
        case OP_SET_GLOBAL: return "SET_GLOBAL";
        case OP_CREATE_ARRAY: return "CREATE_ARRAY";
        case OP_GET_INDEX: return "GET_INDEX";
        case OP_SET_INDEX: return "SET_INDEX";
        case OP_CREATE_STRING: return "CREATE_STRING";
        case OP_CALL: return "CALL";
        case OP_RETURN: return "RETURN";
        case OP_HALT: return "HALT";
        default: return "UNKNOWN";
    }
}

void disassemble_chunk(BytecodeChunk* chunk) {
    size_t i = 0;
    while (i < chunk->count) {
        printf("%04zu: ", i);
        OpCode op = chunk->bytecode[i++];
        printf("%s", op_name(op));
        
        if (op == OP_CONSTANT_INT || op == OP_CONSTANT_FLOAT) {
            int64_t val = 0;
            for (int j = 0; j < 8; j++) {
                val |= ((int64_t)chunk->bytecode[i++]) << (j * 8);
            }
            printf(" %ld", (long)val);
        } else if (op == OP_GET_LOCAL || op == OP_SET_LOCAL || op == OP_CREATE_ARRAY || op == OP_CALL) {
            printf(" %d", chunk->bytecode[i++]);
        } else if (op == OP_JUMP || op == OP_JUMP_IF_FALSE) {
            int64_t offset = 0;
            for (int j = 0; j < 8; j++) {
                offset |= ((int64_t)chunk->bytecode[i++]) << (j * 8);
            }
            printf(" %ld", (long)offset);
        }
        printf("\n");
    }
}

void compiled_program_print(CompiledProgram* program) {
    printf("=== Bytecode ===\n");
    disassemble_chunk(&program->code);
    printf("\n=== Constants (%zu) ===\n", program->constant_count);
    for (size_t i = 0; i < program->constant_count; i++) {
        printf("[%zu] ", i);
        value_print(&program->constants[i]);
        printf("\n");
    }
}