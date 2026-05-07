#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cli.h"
#include "compiler/lexer/lexer.h"
#include "compiler/version.h"
#include "compiler/parser/parser.h"
#include "compiler/interpreter/interpreter.h"
#include "compiler/vm/compiler.h"
#include "compiler/vm/vm.h"
#include "compiler/i18n/i18n.h"

#define MAX_SOURCE_SIZE (1024 * 1024)
#define MAX_IMPORT_DEPTH 16

/**
 * Resolves an import path relative to a base directory.
 * Returns a newly allocated string with the full path.
 */
static char* resolve_import_path(const char* import_path, const char* base_dir) {
    if (import_path[0] == '/') {
        return strdup(import_path);
    }

    /* Check if it's a module path (no slashes, contains dot - like "std.math") */
    int has_slash = (strchr(import_path, '/') != NULL);
    const char* first_dot = strchr(import_path, '.');

    char* path_to_use = NULL;

    if (!has_slash && first_dot != NULL) {
        /* Module path: std.math -> stdlib/math.hn */
        const char* rest = first_dot + 1;

        /* Build module_path: "stdlib/" + rest + ".hn" */
        size_t rest_len = strlen(rest);
        size_t len = 7 + rest_len + 3 + 1; /* "stdlib/" + rest + ".hn" + null */
        char* module_path = (char*)malloc(len);
        char* out = module_path;

        /* Add "stdlib/" prefix */
        memcpy(out, "stdlib/", 7);
        out += 7;

        /* Copy the rest after the first dot, replacing remaining dots with slashes */
        const char* p = rest;
        while (*p) {
            if (*p == '.') {
                *out++ = '/';
            } else {
                *out++ = *p;
            }
            p++;
        }

        /* Add ".hn" extension */
        memcpy(out, ".hn", 3);
        out += 3;
        *out = '\0';

        path_to_use = module_path;
    } else {
        path_to_use = strdup(import_path);
    }

    /* Try multiple locations */
    /* 1. Relative to current working directory */
    char* cwd = getcwd(NULL, 0);
    if (cwd) {
        size_t full_len = strlen(cwd) + 1 + strlen(path_to_use) + 1;
        char* full = (char*)malloc(full_len);
        snprintf(full, full_len, "%s/%s", cwd, path_to_use);
        free(cwd);
        FILE* test = fopen(full, "r");
        if (test) {
            fclose(test);
            free(path_to_use);
            return full;
        }
        free(full);
    }

    /* 2. Relative to base_dir */
    if (base_dir && base_dir[0]) {
        size_t full_len = strlen(base_dir) + 1 + strlen(path_to_use) + 1;
        char* full = (char*)malloc(full_len);
        snprintf(full, full_len, "%s/%s", base_dir, path_to_use);
        FILE* test = fopen(full, "r");
        if (test) {
            fclose(test);
            free(path_to_use);
            return full;
        }
        free(full);
    }

    /* Fallback: just return the path */
    free(path_to_use);
    size_t len = strlen(base_dir) + 1 + strlen(import_path) + 1;
    char* full = (char*)malloc(len);
    snprintf(full, len, "%s/%s", base_dir, import_path);
    return full;
}

/** Reads a raw file into a string (no import expansion) */
static char* read_raw_file(const char* filename, long* out_size) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* source = (char*)malloc(file_size + 1);
    fread(source, 1, file_size, fp);
    source[file_size] = '\0';
    fclose(fp);
    
    *out_size = file_size;
    return source;
}

/** Forward declaration for recursion */
static char* expand_imports_recursive(const char* source, long source_size,
                                       const char* base_dir, int depth, int* out_size);

/**
 * Expands import statements in source code by recursively loading imported files.
 */
static char* expand_imports_recursive(const char* source, long source_size,
                                        const char* base_dir, int depth, int* out_size) {
    if (depth >= MAX_IMPORT_DEPTH) {
        i18n_error(ERR_IMPORT_DEPTH_EXCEEDED, MAX_IMPORT_DEPTH);
        fprintf(stderr, "\n");
        *out_size = (int)source_size;
        return strdup(source);
    }

    char* output = (char*)malloc(MAX_SOURCE_SIZE);
    size_t out_pos = 0;
    size_t pos = 0;
    size_t file_size = (size_t)source_size;

    while (pos < file_size) {
        /* Skip string literals to avoid false import detection */
        if (source[pos] == '"') {
            size_t end = pos + 1;
            while (end < file_size && source[end] != '"') {
                if (source[end] == '\\' && end + 1 < file_size) {
                    end++; /* skip escaped quote */
                }
                end++;
            }
            if (end < file_size) {
                end++; /* include closing quote */
            }
            size_t len = end - pos;
            if (out_pos + len < MAX_SOURCE_SIZE) {
                memcpy(output + out_pos, source + pos, len);
                out_pos += len;
            }
            pos = end;
            continue;
        }

        /* Check for import keyword at start of line */
        int is_import = 0;
        size_t import_start = pos;

        /* Skip leading whitespace */
        while (import_start < file_size && (source[import_start] == ' ' || source[import_start] == '\t')) {
            import_start++;
        }

        /* Check if "import " starts at import_start */
        if (import_start + 7 <= file_size && strncmp(source + import_start, "import ", 7) == 0) {
            /* Make sure this is at the start of a line (pos is at newline or start of file) */
            if (pos == 0 || source[pos-1] == '\n' || source[pos-1] == '\r') {
                is_import = 1;
            }
        } else if (import_start + 13 <= file_size && strncmp(source + import_start, "импорт ", 13) == 0) {
            if (pos == 0 || source[pos-1] == '\n' || source[pos-1] == '\r') {
                is_import = 1;
            }
        }

        if (is_import) {
            /* Find the import path - it should be after "import " or "импорт " */
            size_t path_start = import_start;
            if (strncmp(source + import_start, "import ", 7) == 0) {
                path_start += 7;
            } else {
                path_start += 13;
            }

            /* Skip whitespace */
            while (path_start < file_size && (source[path_start] == ' ' || source[path_start] == '\t')) {
                path_start++;
            }

            if (path_start < file_size && source[path_start] == '"') {
                /* Quoted path */
                path_start++; /* skip opening quote */
                size_t path_end = path_start;
                while (path_end < file_size && source[path_end] != '"') {
                    path_end++;
                }

                if (path_end < file_size) {
                    size_t path_len = path_end - path_start;
                    char* import_path = (char*)malloc(path_len + 1);
                    memcpy(import_path, source + path_start, path_len);
                    import_path[path_len] = '\0';

                    char* full_path = resolve_import_path(import_path, base_dir);

                    long imp_file_size = 0;
                    char* imp_raw = read_raw_file(full_path, &imp_file_size);

                    if (imp_raw) {
                        char* imp_dir_copy = strdup(full_path);
                        char* imp_last_slash = strrchr(imp_dir_copy, '/');
                        char* imp_base_dir = "";
                        if (imp_last_slash) {
                            *imp_last_slash = '\0';
                            imp_base_dir = imp_dir_copy;
                        }

                        int expanded_size = 0;
                        char* expanded = expand_imports_recursive(imp_raw, imp_file_size,
                                                                   imp_base_dir, depth + 1,
                                                                   &expanded_size);

                        if (out_pos + expanded_size + 1 < MAX_SOURCE_SIZE) {
                            memcpy(output + out_pos, expanded, expanded_size);
                            out_pos += expanded_size;
                            output[out_pos++] = '\n';
                        }

                        free(expanded);
                        free(imp_dir_copy);
                        free(imp_raw);
                    } else {
                        fprintf(stderr, "Cannot open imported file '%s'\n", full_path);
                    }

                    free(full_path);
                    free(import_path);

                    pos = path_end + 1;
                    /* Skip to end of line */
                    while (pos < file_size && source[pos] != '\n' && source[pos] != ';') {
                        pos++;
                    }
                    if (pos < file_size && source[pos] == ';') {
                        pos++;
                    }
                    continue;
                }
            } else {
                /* Module path (no quotes): import std.math */
                size_t path_end = path_start;
                while (path_end < file_size && source[path_end] != '\n' && source[path_end] != ';' && source[path_end] != ' ') {
                    path_end++;
                }

                size_t path_len = path_end - path_start;
                char* import_path = (char*)malloc(path_len + 1);
                memcpy(import_path, source + path_start, path_len);
                import_path[path_len] = '\0';

                char* full_path = resolve_import_path(import_path, base_dir);

                long imp_file_size = 0;
                char* imp_raw = read_raw_file(full_path, &imp_file_size);

                if (imp_raw) {
                    char* imp_dir_copy = strdup(full_path);
                    char* imp_last_slash = strrchr(imp_dir_copy, '/');
                    char* imp_base_dir = "";
                    if (imp_last_slash) {
                        *imp_last_slash = '\0';
                        imp_base_dir = imp_dir_copy;
                    }

                    int expanded_size = 0;
                    char* expanded = expand_imports_recursive(imp_raw, imp_file_size,
                                                               imp_base_dir, depth + 1,
                                                               &expanded_size);

                    if (out_pos + expanded_size + 1 < MAX_SOURCE_SIZE) {
                        memcpy(output + out_pos, expanded, expanded_size);
                        out_pos += expanded_size;
                        output[out_pos++] = '\n';
                    }

                    free(expanded);
                    free(imp_dir_copy);
                    free(imp_raw);
                } else {
                    fprintf(stderr, "Cannot open imported file '%s'\n", full_path);
                }

                free(full_path);
                free(import_path);

                pos = path_end;
                while (pos < file_size && source[pos] != '\n' && source[pos] != ';') {
                    pos++;
                }
                if (pos < file_size && source[pos] == ';') {
                    pos++;
                }
                continue;
            }
        }

        if (out_pos < MAX_SOURCE_SIZE - 1) {
            output[out_pos++] = source[pos];
        }
        pos++;
    }

    output[out_pos] = '\0';
    *out_size = (int)out_pos;
    return output;
}

/**
 * Reads a file and resolves import statements by loading included files.
 * Returns the combined source with imports expanded.
 */
static char* read_source_with_imports(const char* filename, int* out_size) {
    long file_size = 0;
    char* source = read_raw_file(filename, &file_size);
    if (!source) {
        i18n_error(ERR_CANNOT_OPEN_FILE, filename);
        fprintf(stderr, "\n");
        return NULL;
    }
    
    /* Get the directory of the file for resolving imports */
    char* dir_copy = strdup(filename);
    char* last_slash = strrchr(dir_copy, '/');
    char* base_dir = "";
    if (last_slash) {
        *last_slash = '\0';
        base_dir = dir_copy;
    }
    
    char* result = expand_imports_recursive(source, file_size, base_dir, 0, out_size);
    
    free(source);
    free(dir_copy);
    return result;
}

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