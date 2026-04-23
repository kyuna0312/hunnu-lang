#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "compiler/interpreter/interpreter.h"

void print_usage(const char* prog_name) {
    printf("Usage: %s <command> [options] <file>\n", prog_name);
    printf("\nCommands:\n");
    printf("  run <file.hn>   Run a Hunnu source file\n");
    printf("  build <file.hn> Compile a Hunnu source file to bytecode\n");
    printf("  tokens <file>   Print tokens (for debugging)\n");
    printf("  ast <file>     Print AST (for debugging)\n");
    printf("\nOptions:\n");
    printf("  -v, --version   Show version information\n");
    printf("  -h, --help     Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s run examples/main.hn\n", prog_name);
    printf("  %s build examples/main.hn\n", prog_name);
}

void print_version(void) {
    printf("hunnu version 0.1.0 (MVP)\n");
    printf("A simple, safe, and fast programming language\n");
}

int cmd_run(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* source = (char*)malloc(file_size + 1);
    fread(source, 1, file_size, fp);
    source[file_size] = '\0';
    fclose(fp);

    Lexer* lexer = lexer_new(source);
    ASTNode* program = parse(lexer);
    
    if (!program) {
        fprintf(stderr, "Error: Failed to parse source file\n");
        free(source);
        lexer_free(lexer);
        return 1;
    }

    Interpreter* interp = interpreter_new();
    int result = interpreter_run(interp, program);
    
    interpreter_free(interp);
    ast_free(program);
    lexer_free(lexer);
    free(source);

    return result;
}

int cmd_tokens(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* source = (char*)malloc(file_size + 1);
    fread(source, 1, file_size, fp);
    source[file_size] = '\0';
    fclose(fp);

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
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* source = (char*)malloc(file_size + 1);
    fread(source, 1, file_size, fp);
    source[file_size] = '\0';
    fclose(fp);

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
            fprintf(stderr, "Error: Missing filename argument\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_run(argv[2]);
    }

    if (strcmp(argv[1], "tokens") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing filename argument\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_tokens(argv[2]);
    }

    if (strcmp(argv[1], "ast") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing filename argument\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_ast(argv[2]);
    }

    if (strcmp(argv[1], "build") == 0) {
        fprintf(stderr, "Error: 'build' command not yet implemented\n");
        return 1;
    }

    fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}