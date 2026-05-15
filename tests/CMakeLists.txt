# Test executable
add_executable(hunnu_tests
    main.c
    test_lexer.c
    test_parser.c
    test_value.c
    test_scope.c
    test_interpreter.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/lexer/lexer.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/lexer/token.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/parser/parser.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/ast/ast.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/ast/ast_free.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/ast/ast_print.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/interpreter/interpreter.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/interpreter/builtins.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/value.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/scope.c
    ${CMAKE_SOURCE_DIR}/compiler-core/compiler/i18n/i18n.c
)

target_include_directories(hunnu_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(hunnu_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(hunnu_tests dl)

add_test(NAME hunnu_c_unit_tests COMMAND hunnu_tests)
