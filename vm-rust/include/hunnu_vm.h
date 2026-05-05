#ifndef HUNNU_VM_H
#define HUNNU_VM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run Hunnu bytecode using the Rust VM
 * 
 * @param bytecode Pointer to bytecode array
 * @param bytecode_len Length of bytecode array
 * @param constants Pointer to constants data (binary format)
 * @param constants_len Length of constants data
 * @return 0 on success, -1 on error
 */
int hunnu_vm_run(const uint8_t* bytecode, size_t bytecode_len,
                 const uint8_t* constants, size_t constants_len);

#ifdef __cplusplus
}
#endif

#endif /* HUNNU_VM_H */
