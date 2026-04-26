/**
 * @file vm.h
 * @brief Virtual machine declarations
 */

#ifndef HUNNU_VM_H
#define HUNNU_VM_H

#include "compiler.h"

/**
 * @brief Executes compiled bytecode
 * @param program Compiled program
 * @return 0 on success, non-zero on error
 */
int vm_execute(CompiledProgram* program);

#endif