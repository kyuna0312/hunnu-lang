#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "opcodes.h"
#include "../interpreter/interpreter.h"

#define STACK_MAX 256
#define MAX_LOCALS 256

typedef struct {
    Value stack[STACK_MAX];
    int stack_count;
    
    Value locals[MAX_LOCALS];
    int local_count;
    
    CompiledProgram* program;
    size_t ip;
} VM;

static void vm_init(VM* vm) {
    vm->stack_count = 0;
    vm->local_count = 0;
    vm->program = NULL;
    vm->ip = 0;
}

static void vm_push(VM* vm, Value value) {
    if (vm->stack_count >= STACK_MAX) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }
    vm->stack[vm->stack_count++] = value;
}

static Value vm_pop(VM* vm) {
    if (vm->stack_count <= 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }
    return vm->stack[--vm->stack_count];
}

static Value* vm_peek(VM* vm) {
    if (vm->stack_count <= 0) return NULL;
    return &vm->stack[vm->stack_count - 1];
}

static int vm_get_bool(Value* v) {
    if (v->type == VALUE_BOOL) return v->value.bool_value;
    if (v->type == VALUE_INT) return v->value.int_value != 0;
    if (v->type == VALUE_FLOAT) return v->value.float_value != 0.0;
    return 1;
}

static void vm_binary_op(VM* vm, OpCode op) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    
    if (a.type == VALUE_INT && b.type == VALUE_INT) {
        int64_t result;
        switch (op) {
            case OP_ADD: result = a.value.int_value + b.value.int_value; break;
            case OP_SUBTRACT: result = a.value.int_value - b.value.int_value; break;
            case OP_MULTIPLY: result = a.value.int_value * b.value.int_value; break;
            case OP_DIVIDE: result = a.value.int_value / b.value.int_value; break;
            case OP_MODULO: result = a.value.int_value % b.value.int_value; break;
            default: result = 0; break;
        }
        Value r = value_create_int(result);
        vm_push(vm, r);
    } else {
        double af = a.type == VALUE_FLOAT ? a.value.float_value : (double)a.value.int_value;
        double bf = b.type == VALUE_FLOAT ? b.value.float_value : (double)b.value.int_value;
        double result;
        switch (op) {
            case OP_ADD: result = af + bf; break;
            case OP_SUBTRACT: result = af - bf; break;
            case OP_MULTIPLY: result = af * bf; break;
            case OP_DIVIDE: result = af / bf; break;
            case OP_MODULO: result = (int64_t)af % (int64_t)bf; break;
            default: result = 0; break;
        }
        Value r = value_create_float(result);
        vm_push(vm, r);
    }
}

static void vm_compare_op(VM* vm, OpCode op) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    
    int result = 0;
    
    if (a.type == VALUE_INT && b.type == VALUE_INT) {
        switch (op) {
            case OP_GREATER: result = a.value.int_value > b.value.int_value; break;
            case OP_LESS: result = a.value.int_value < b.value.int_value; break;
            case OP_GREATER_EQUAL: result = a.value.int_value >= b.value.int_value; break;
            case OP_LESS_EQUAL: result = a.value.int_value <= b.value.int_value; break;
            case OP_EQUAL: result = a.value.int_value == b.value.int_value; break;
            case OP_NOT_EQUAL: result = a.value.int_value != b.value.int_value; break;
            default: result = 0; break;
        }
    } else if (a.type == VALUE_FLOAT || b.type == VALUE_FLOAT) {
        double af = a.type == VALUE_FLOAT ? a.value.float_value : (double)a.value.int_value;
        double bf = b.type == VALUE_FLOAT ? b.value.float_value : (double)b.value.int_value;
        switch (op) {
            case OP_GREATER: result = af > bf; break;
            case OP_LESS: result = af < bf; break;
            case OP_GREATER_EQUAL: result = af >= bf; break;
            case OP_LESS_EQUAL: result = af <= bf; break;
            case OP_EQUAL: result = af == bf; break;
            case OP_NOT_EQUAL: result = af != bf; break;
            default: result = 0; break;
        }
    } else if (a.type == VALUE_STRING && b.type == VALUE_STRING) {
        int cmp = strcmp(a.value.string_value, b.value.string_value);
        switch (op) {
            case OP_EQUAL: result = cmp == 0; break;
            case OP_NOT_EQUAL: result = cmp != 0; break;
            default: result = 0; break;
        }
    } else if (a.type == VALUE_BOOL && b.type == VALUE_BOOL) {
        switch (op) {
            case OP_EQUAL: result = a.value.bool_value == b.value.bool_value; break;
            case OP_NOT_EQUAL: result = a.value.bool_value != b.value.bool_value; break;
            default: result = 0; break;
        }
    }
    
    Value r = value_create_bool(result);
    vm_push(vm, r);
}

static int64_t read_int64(uint8_t* bytecode, size_t* ip) {
    int64_t val = 0;
    for (int i = 0; i < 8; i++) {
        val |= ((int64_t)bytecode[(*ip)++]) << (i * 8);
    }
    return val;
}

static void vm_call_builtin(VM* vm, const char* name, int arg_count) {
    if (strcmp(name, "print") == 0) {
        for (int i = 0; i < arg_count; i++) {
            Value v = vm_pop(vm);
            value_print(&v);
        }
        printf("\n");
    } else if (strcmp(name, "input") == 0) {
        char buf[1024];
        if (fgets(buf, sizeof(buf), stdin)) {
            buf[strcspn(buf, "\n")] = 0;
            Value v = value_create_string(buf);
            vm_push(vm, v);
        }
    } else if (strcmp(name, "to_int") == 0) {
        Value v = vm_pop(vm);
        Value result;
        if (v.type == VALUE_STRING) {
            result = value_create_int(atoi(v.value.string_value));
        } else if (v.type == VALUE_FLOAT) {
            result = value_create_int((int64_t)v.value.float_value);
        } else {
            result = value_create_int(v.value.int_value);
        }
        vm_push(vm, result);
    } else if (strcmp(name, "to_float") == 0) {
        Value v = vm_pop(vm);
        Value result;
        if (v.type == VALUE_STRING) {
            result = value_create_float(atof(v.value.string_value));
        } else if (v.type == VALUE_INT) {
            result = value_create_float((double)v.value.int_value);
        } else {
            result = value_create_float(v.value.float_value);
        }
        vm_push(vm, result);
    } else if (strcmp(name, "to_str") == 0) {
        Value v = vm_pop(vm);
        char buf[64];
        if (v.type == VALUE_INT) {
            snprintf(buf, sizeof(buf), "%lld", (long long)v.value.int_value);
        } else if (v.type == VALUE_FLOAT) {
            snprintf(buf, sizeof(buf), "%f", v.value.float_value);
        } else if (v.type == VALUE_BOOL) {
            snprintf(buf, sizeof(buf), "%s", v.value.bool_value ? "true" : "false");
        } else {
            snprintf(buf, sizeof(buf), "null");
        }
        Value result = value_create_string(buf);
        vm_push(vm, result);
    } else {
        fprintf(stderr, "Unknown builtin: %s\n", name);
    }
}

static void vm_run(VM* vm) {
    uint8_t* bytecode = vm->program->code.bytecode;
    size_t count = vm->program->code.count;
    
    while (vm->ip < count) {
        OpCode op = bytecode[vm->ip++];
        
        switch (op) {
            case OP_CONSTANT_INT: {
                int64_t val = read_int64(bytecode, &vm->ip);
                Value v = value_create_int(val);
                vm_push(vm, v);
                break;
            }
            
            case OP_CONSTANT_FLOAT: {
                int64_t bits = read_int64(bytecode, &vm->ip);
                double val = *(double*)&bits;
                Value v = value_create_float(val);
                vm_push(vm, v);
                break;
            }
            
            case OP_CONSTANT_STRING: {
                uint8_t idx = bytecode[vm->ip++];
                Value v = vm->program->constants[idx];
                vm_push(vm, v);
                break;
            }
            
            case OP_NIL: {
                Value v = value_create_none();
                vm_push(vm, v);
                break;
            }
            
            case OP_TRUE: {
                Value v = value_create_bool(1);
                vm_push(vm, v);
                break;
            }
            
            case OP_FALSE: {
                Value v = value_create_bool(0);
                vm_push(vm, v);
                break;
            }
            
            case OP_ADD:
            case OP_SUBTRACT:
            case OP_MULTIPLY:
            case OP_DIVIDE:
            case OP_MODULO: {
                vm_binary_op(vm, op);
                break;
            }
            
            case OP_GREATER:
            case OP_LESS:
            case OP_GREATER_EQUAL:
            case OP_LESS_EQUAL:
            case OP_EQUAL:
            case OP_NOT_EQUAL: {
                vm_compare_op(vm, op);
                break;
            }
            
            case OP_NOT: {
                Value v = vm_pop(vm);
                Value r = value_create_bool(!vm_get_bool(&v));
                vm_push(vm, r);
                break;
            }
            
            case OP_NEGATE: {
                Value v = vm_pop(vm);
                Value r;
                if (v.type == VALUE_INT) {
                    r = value_create_int(-v.value.int_value);
                } else {
                    r = value_create_float(-v.value.float_value);
                }
                vm_push(vm, r);
                break;
            }
            
            case OP_JUMP: {
                int64_t offset = read_int64(bytecode, &vm->ip);
                vm->ip += offset;
                break;
            }
            
            case OP_JUMP_IF_FALSE: {
                int64_t offset = read_int64(bytecode, &vm->ip);
                Value* top = vm_peek(vm);
                if (!vm_get_bool(top)) {
                    vm->ip += offset;
                }
                break;
            }
            
            case OP_POP: {
                vm_pop(vm);
                break;
            }
            
            case OP_GET_LOCAL: {
                uint8_t idx = bytecode[vm->ip++];
                if (idx < vm->local_count) {
                    vm_push(vm, vm->locals[idx]);
                } else {
                    Value v = value_create_none();
                    vm_push(vm, v);
                }
                break;
            }
            
            case OP_SET_LOCAL: {
                uint8_t idx = bytecode[vm->ip++];
                Value* v_ptr = vm_peek(vm);
                if (idx >= MAX_LOCALS) break;
                if (idx >= vm->local_count) {
                    vm->local_count = idx + 1;
                }
                vm->locals[idx] = *v_ptr;
                break;
            }
            
            case OP_CREATE_ARRAY: {
                uint8_t count = bytecode[vm->ip++];
                Value Array = value_create_array(count);
                for (int i = count - 1; i >= 0; i--) {
                    Array.array_elements[i] = (Value*)malloc(sizeof(Value));
                    *Array.array_elements[i] = vm_pop(vm);
                }
                vm_push(vm, Array);
                break;
            }
            
            case OP_GET_INDEX: {
                Value index = vm_pop(vm);
                Value arr = vm_pop(vm);
                if (arr.type != VALUE_ARRAY) break;
                int64_t idx = index.type == VALUE_FLOAT ? (int64_t)index.value.float_value : index.value.int_value;
                if (idx < 0) idx += arr.array_length;
                if (idx >= 0 && idx < (int64_t)arr.array_length) {
                    vm_push(vm, *arr.array_elements[idx]);
                } else {
                    Value v = value_create_none();
                    vm_push(vm, v);
                }
                break;
            }
            
            case OP_CALL: {
                uint8_t arg_count = bytecode[vm->ip++];
                Value fn = vm_pop(vm);
                if (fn.type == VALUE_STRING) {
                    vm_call_builtin(vm, fn.value.string_value, arg_count);
                }
                break;
            }
            
            case OP_RETURN: {
                return;
            }
            
            case OP_HALT: {
                return;
            }
            
            default:
                break;
        }
    }
}

int vm_execute(CompiledProgram* program) {
    VM vm;
    vm_init(&vm);
    vm.program = program;
    vm_run(&vm);
    return 0;
}