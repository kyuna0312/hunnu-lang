#ifndef HUNNU_RUST_COMPILER_H
#define HUNNU_RUST_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Lex source code and return token debug output as a C string */
char* hunnu_rust_lex(const char* source);

/* Parse source code and return AST debug output as a C string */
char* hunnu_rust_parse(const char* source);

/* Free a string returned by hunnu_rust_lex or hunnu_rust_parse */
void hunnu_rust_free_string(char* s);

#ifdef __cplusplus
}
#endif

#endif /* HUNNU_RUST_COMPILER_H */
