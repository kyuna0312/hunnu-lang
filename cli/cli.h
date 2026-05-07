#ifndef HUNNU_CLI_H
#define HUNNU_CLI_H

void print_usage(const char* prog_name);
void print_version(void);
int cmd_run(const char* filename, int debug, int use_vm, int use_vm_rust);
int cmd_tokens(const char* filename);
int cmd_ast(const char* filename);

#endif