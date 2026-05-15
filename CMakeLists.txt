cmake_minimum_required(VERSION 3.15)
project(hunnu_lang VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_RUST_VM "Build Rust VM component" ON)
option(BUILD_RUST_COMPILER "Build Rust compiler frontend" ON)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/compiler-core)
include_directories(${CMAKE_SOURCE_DIR}/compiler-core/compiler)
include_directories(${CMAKE_SOURCE_DIR}/vm-rust/include)
include_directories(${CMAKE_SOURCE_DIR}/compiler-core/compiler-rust/include)

# C sources for the main hunnu binary
set(HUNNU_SOURCES
    cli/main.c
    cli/cli.c
    compiler-core/compiler/lexer/lexer.c
    compiler-core/compiler/lexer/token.c
    compiler-core/compiler/parser/parser.c
    compiler-core/compiler/ast/ast.c
    compiler-core/compiler/ast/ast_free.c
    compiler-core/compiler/ast/ast_print.c
    compiler-core/compiler/interpreter/interpreter.c
    compiler-core/compiler/interpreter/builtins.c
    compiler-core/compiler/value.c
    compiler-core/compiler/scope.c
    compiler-core/compiler/import.c
    compiler-core/compiler/i18n/i18n.c
    compiler-core/compiler/version.c
    compiler-core/compiler/vm/compiler.c
    compiler-core/compiler/vm/vm.c
    compiler-core/compiler/transpile/transpile.c
)

# Rust VM integration
if(BUILD_RUST_VM)
    find_program(CARGO_EXECUTABLE cargo)
    if(CARGO_EXECUTABLE)
        message(STATUS "Found cargo: ${CARGO_EXECUTABLE}")

        # Build Rust VM as a static library
        set(RUST_VM_LIB ${CMAKE_SOURCE_DIR}/vm-rust/target/release/libhunnu_vm.a)
        set(RUST_VM_HEADER ${CMAKE_SOURCE_DIR}/vm-rust/include/hunnu_vm.h)

        # Custom command to build the Rust VM
        add_custom_command(
            OUTPUT ${RUST_VM_LIB}
            COMMAND ${CARGO_EXECUTABLE} build --release
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/vm-rust
            COMMENT "Building Rust VM library"
        )

        # Custom target for the Rust VM
        add_custom_target(hunnu_vm_rust DEPENDS ${RUST_VM_LIB})

        # Build the C components with Rust VM support
        add_executable(hunnu ${HUNNU_SOURCES})

        # Link with Rust VM library
        target_link_libraries(hunnu ${RUST_VM_LIB})
        target_link_libraries(hunnu pthread dl)

        # Make hunnu depend on the Rust VM build
        add_dependencies(hunnu hunnu_vm_rust)

        # Install the Rust VM binary (optional, for standalone use)
        install(PROGRAMS ${CMAKE_SOURCE_DIR}/vm-rust/target/release/hunnu-vm
                DESTINATION bin
                RENAME hunnu-vm
                OPTIONAL)

        message(STATUS "Rust VM will be linked with hunnu binary")
    else()
        message(WARNING "cargo not found - building without Rust VM support")
        add_executable(hunnu ${HUNNU_SOURCES})
    endif()
else()
    add_executable(hunnu ${HUNNU_SOURCES})
endif()

target_compile_options(hunnu PRIVATE -Wall -Wextra -pedantic)

# Install the C binary
install(TARGETS hunnu DESTINATION bin)

# Testing
enable_testing()

# C unit tests
add_subdirectory(tests)

# C interpreter integration tests
add_test(NAME hunnu_version COMMAND hunnu -v)

# Integration test suite
add_test(NAME hunnu_tests
    COMMAND ${CMAKE_SOURCE_DIR}/run_tests.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Rust tests
if(BUILD_RUST_VM AND CARGO_EXECUTABLE)
    add_test(
        NAME rust_vm_tests
        COMMAND ${CARGO_EXECUTABLE} test
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/vm-rust
    )
endif()

if(BUILD_RUST_COMPILER AND CARGO_EXECUTABLE)
    add_test(
        NAME rust_compiler_tests
        COMMAND ${CARGO_EXECUTABLE} test
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/compiler-core/compiler-rust
    )
endif()

message(STATUS "Build configuration:")
message(STATUS "  C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(BUILD_RUST_VM AND CARGO_EXECUTABLE)
    message(STATUS "  Rust VM: enabled")
else()
    message(STATUS "  Rust VM: disabled")
endif()

# Rust compiler frontend integration
if(BUILD_RUST_COMPILER)
    if(CARGO_EXECUTABLE)
        set(RUST_COMPILER_LIB ${CMAKE_SOURCE_DIR}/compiler-core/compiler-rust/target/release/libhunnu_compiler.a)
        set(RUST_COMPILER_HEADER ${CMAKE_SOURCE_DIR}/compiler-core/compiler-rust/include/hunnu_compiler.h)

        add_custom_command(
            OUTPUT ${RUST_COMPILER_LIB}
            COMMAND ${CARGO_EXECUTABLE} build --release
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/compiler-core/compiler-rust
            COMMENT "Building Rust compiler library"
        )

        add_custom_target(hunnu_compiler_rust DEPENDS ${RUST_COMPILER_LIB})

        if(TARGET hunnu)
            target_link_libraries(hunnu ${RUST_COMPILER_LIB})
            add_dependencies(hunnu hunnu_compiler_rust)
        endif()

        install(PROGRAMS ${CMAKE_SOURCE_DIR}/compiler-core/compiler-rust/target/release/hunnu-compiler
                DESTINATION bin
                OPTIONAL)

        if(TARGET hunnu)
            target_compile_definitions(hunnu PRIVATE HUNNU_HAVE_RUST_COMPILER=1)
        endif()
        message(STATUS "  Rust compiler: enabled")
    else()
        message(STATUS "  Rust compiler: disabled (cargo not found)")
    endif()
endif()
