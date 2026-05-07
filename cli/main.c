#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "compiler/import.h"
#include "compiler/lexer/lexer.h"
#include "compiler/version.h"
#include "compiler/parser/parser.h"
#include "compiler/interpreter/interpreter.h"
#include "compiler/vm/compiler.h"
#include "compiler/vm/vm.h"
#include "compiler/i18n/i18n.h"

void print_usage(const char* prog_name) {
    printf("Usage: %s <command> [options] <file>\n", prog_name);
    printf("\nCommands:\n");
    printf("  run <file.hn>   Run a Hunnu source file\n");
    printf("  build <file.hn> Compile a Hunnu source file to bytecode\n");
    printf("  compile <file.hn> Compile to native binary (Month 3 - AOT)\n");
    printf("  tokens <file>   Print tokens (for debugging)\n");
    printf("  ast <file>     Print AST (for debugging)\n");
    printf("\nOptions:\n");
    printf("  -v, --version   Show version information\n");
    printf("  -h, --help     Show this help message\n");
    printf("  --vm           Use C bytecode VM (for run command)\n");
    printf("  --vm-rust      Use Rust bytecode VM (for run command)\n");
    printf("  --debug        Enable debug output (for run command)\n");
    printf("\nExamples:\n");
    printf("  %s run examples/main.hn\n", prog_name);
    printf("  %s run examples/main.hn --debug\n", prog_name);
    printf("  %s build examples/main.hn\n", prog_name);
    printf("  %s compile examples/main.hn -o main\n", prog_name);
    printf("\nLanguage Features:\n");
    printf("  Variables: let x = 10\n");
    printf("  Functions: fn add(a, b) { return a + b }\n");
    printf("  Control: if/else, while, for loops\n");
    printf("  Arrays: let arr = [1, 2, 3]\n");
    printf("  Structs: type Point = { x: int, y: int }\n");
}

void print_version(void) {
    printf("hunnu %s\n", version_get_string());
    printf("%s\n", version_get_description());
}

int cmd_run(const char* filename, int debug, int use_vm, int use_vm_rust) {
    int source_size = 0;
    char* source = read_source_with_imports(filename, &source_size);
    if (!source) {
        return 1;
    }

    Lexer* lexer = lexer_new(source);
    
    if (debug) {
        printf("--- Tokens ---\n");
        Token* token;
        while ((token = lexer_next_token(lexer)) != NULL) {
            if (token->type == TOKEN_EOF) break;
            token_print(token);
            printf("\n");
        }
        printf("--- End Tokens ---\n\n");
        
        lexer_free(lexer);
        lexer = lexer_new(source);
    }
    
    ASTNode* program = parse(lexer);
    
    if (!program) {
        i18n_error(ERR_FAILED_PARSE);
        fprintf(stderr, "\n");
        free(source);
        lexer_free(lexer);
        return 1;
    }
    
    if (debug) {
        printf("--- AST ---\n");
        ast_print(program, 0);
        printf("--- End AST ---\n\n");
    }

    int result;
    
    if (use_vm_rust) {
        // Compile to bytecode and run with Rust VM
        CompiledProgram* compiled = compile_program(program);
        
        // Serialize bytecode and constants to memory buffers
        // First, write bytecode to buffer
        size_t bytecode_len = compiled->code.count;
        uint8_t* bytecode_buf = malloc(bytecode_len);
        if (!bytecode_buf) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            compiled_program_free(compiled);
            ast_free(program);
            lexer_free(lexer);
            free(source);
            return 1;
        }
        memcpy(bytecode_buf, compiled->code.bytecode, bytecode_len);
        
        // Serialize constants to buffer
        // Format: 8 bytes (count) + each constant
        size_t constants_size = 8;  // count
        for (size_t i = 0; i < compiled->constant_count; i++) {
            constants_size += 1;  // type byte
            switch (compiled->constants[i].type) {
                case VALUE_INT:
                case VALUE_FLOAT:
                    constants_size += 8;
                    break;
                case VALUE_STRING:
                    constants_size += 8 + strlen(compiled->constants[i].value.string_value);
                    break;
                case VALUE_BOOL:
                    constants_size += 1;
                    break;
                case VALUE_NONE:
                    break;
                case VALUE_ARRAY:
                    constants_size += 8;  // length only for now
                    break;
            }
        }
        
        uint8_t* constants_buf = malloc(constants_size);
        if (!constants_buf) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(bytecode_buf);
            compiled_program_free(compiled);
            ast_free(program);
            lexer_free(lexer);
            free(source);
            return 1;
        }
        
        // Write constants count
        uint64_t count = (uint64_t)compiled->constant_count;
        memcpy(constants_buf, &count, 8);
        size_t pos = 8;
        
        for (size_t i = 0; i < compiled->constant_count; i++) {
            Value* v = &compiled->constants[i];
            uint8_t type_byte;
            
            switch (v->type) {
                case VALUE_INT:
                    type_byte = 0;
                    constants_buf[pos++] = type_byte;
                    memcpy(&constants_buf[pos], &v->value.int_value, 8);
                    pos += 8;
                    break;
                case VALUE_FLOAT:
                    type_byte = 1;
                    constants_buf[pos++] = type_byte;
                    memcpy(&constants_buf[pos], &v->value.float_value, 8);
                    pos += 8;
                    break;
                case VALUE_STRING: {
                    type_byte = 2;
                    constants_buf[pos++] = type_byte;
                    size_t str_len = strlen(v->value.string_value);
                    uint64_t len = (uint64_t)str_len;
                    memcpy(&constants_buf[pos], &len, 8);
                    pos += 8;
                    memcpy(&constants_buf[pos], v->value.string_value, str_len);
                    pos += str_len;
                    break;
                }
                case VALUE_BOOL:
                    type_byte = 3;
                    constants_buf[pos++] = type_byte;
                    constants_buf[pos++] = v->value.bool_value ? 1 : 0;
                    break;
                case VALUE_NONE:
                    type_byte = 4;
                    constants_buf[pos++] = type_byte;
                    break;
                case VALUE_ARRAY:
                    type_byte = 5;
                    constants_buf[pos++] = type_byte;
                    uint64_t arr_len = (uint64_t)v->array_length;
                    memcpy(&constants_buf[pos], &arr_len, 8);
                    pos += 8;
                    break;
            }
        }
        
        // Call Rust VM via FFI
        extern int hunnu_vm_run(const uint8_t*, size_t, const uint8_t*, size_t);
        result = hunnu_vm_run(bytecode_buf, bytecode_len, constants_buf, constants_size);
        
        free(bytecode_buf);
        free(constants_buf);
        compiled_program_free(compiled);
    } else if (use_vm) {
        CompiledProgram* compiled = compile_program(program);
        if (debug) {
            compiled_program_print(compiled);
        }
        result = vm_execute(compiled);
        compiled_program_free(compiled);
    } else {
        Interpreter* interp = interpreter_new();
        result = interpreter_run(interp, program);
        interpreter_free(interp);
    }
    
    ast_free(program);
    lexer_free(lexer);
    free(source);
    
    return result;
}

int cmd_tokens(const char* filename) {
    int source_size = 0;
    char* source = read_source_with_imports(filename, &source_size);
    if (!source) {
        return 1;
    }

    Lexer* lexer = lexer_new(source);
    Token* token;
    
    printf("Tokens:\n");
    printf("-------\n");
    while ((token = lexer_next_token(lexer)) != NULL) {
        if (token->type == TOKEN_EOF) break;
        token_print(token);
        printf("\n");
    }
    
    lexer_free(lexer);
    free(source);
    return 0;
}

int cmd_ast(const char* filename) {
    int source_size = 0;
    char* source = read_source_with_imports(filename, &source_size);
    if (!source) {
        return 1;
    }

    Lexer* lexer = lexer_new(source);
    ASTNode* program = parse(lexer);
    
    if (!program) {
        fprintf(stderr, "Error: Failed to parse source file\n");
        free(source);
        lexer_free(lexer);
        return 1;
    }

    printf("AST:\n");
    printf("---\n");
    ast_print(program, 0);
    
    ast_free(program);
    lexer_free(lexer);
    free(source);
    return 0;
}

int main(int argc, char* argv[]) {
    i18n_init();

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        print_version();
        return 0;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            i18n_error(ERR_MISSING_FILENAME);
            fprintf(stderr, "\n");
            print_usage(argv[0]);
            return 1;
        }
        int debug = 0;
        int use_vm = 0;
        int use_vm_rust = 0;
        const char* filename = NULL;
        
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
                debug = 1;
            } else if (strcmp(argv[i], "--vm") == 0) {
                use_vm = 1;
            } else if (strcmp(argv[i], "--vm-rust") == 0) {
                use_vm_rust = 1;
            } else if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc) {
                i18n_set_language(argv[++i]);
            } else if (argv[i][0] != '-') {
                filename = argv[i];
            }
        }
        
        if (!filename) {
            i18n_error(ERR_MISSING_FILENAME);
            fprintf(stderr, "\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_run(filename, debug, use_vm, use_vm_rust);
    }

    if (strcmp(argv[1], "tokens") == 0) {
        if (argc < 3) {
            i18n_error(ERR_MISSING_FILENAME);
            fprintf(stderr, "\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_tokens(argv[2]);
    }

    if (strcmp(argv[1], "ast") == 0) {
        if (argc < 3) {
            i18n_error(ERR_MISSING_FILENAME);
            fprintf(stderr, "\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_ast(argv[2]);
    }

    if (strcmp(argv[1], "build") == 0) {
        if (argc < 3) {
            i18n_error(ERR_MISSING_FILENAME);
            fprintf(stderr, "\n");
            print_usage(argv[0]);
            return 1;
        }
        
        const char* filename = argv[2];
        int source_size = 0;
        char* source = read_source_with_imports(filename, &source_size);
        if (!source) {
            return 1;
        }

        Lexer* lexer = lexer_new(source);
        ASTNode* program = parse(lexer);
        lexer_free(lexer);

    if (!program) {
        i18n_error(ERR_FAILED_PARSE);
        fprintf(stderr, "\n");
        free(source);
        return 1;
    }

        CompiledProgram* compiled = compile_program(program);
        compiled_program_print(compiled);
        
        compiled_program_free(compiled);
        ast_free(program);
        free(source);
        
        return 0;
    }

    if (strcmp(argv[1], "compile") == 0) {
        if (argc < 3) {
            i18n_error(ERR_MISSING_FILENAME);
            fprintf(stderr, "\n");
            print_usage(argv[0]);
            return 1;
        }
        
        const char* filename = argv[2];
        const char* output = "a.out";
        
        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
                output = argv[++i];
            }
        }
        
        printf("Compiling %s to %s (Month 3 AOT Compiler)...\n", filename, output);
        printf("Note: Full LLVM codegen is in progress (compiler-rust/).\n");
        printf("For now, using C compiler as fallback...\n");
        
        /* Use system C compiler as temporary solution */
        char command[512];
        snprintf(command, sizeof(command), "gcc -o %s -x c - -I. 2>&1", output);
        
        FILE* fp = popen(command, "w");
        if (!fp) {
            fprintf(stderr, "Error: Failed to run compiler\n");
            return 1;
        }
        
        /* Read source and write to compiler */
        int source_size = 0;
        char* source = read_source_with_imports(filename, &source_size);
        if (source) {
            fwrite(source, 1, source_size, fp);
            free(source);
        }
        
        pclose(fp);
        printf("Compilation complete: %s\n", output);
        
        return 0;
    }

    i18n_error(ERR_UNKNOWN_COMMAND, argv[1]);
    fprintf(stderr, "\n");
    print_usage(argv[0]);
    return 1;
}