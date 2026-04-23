cmake_minimum_required(VERSION 3.15)
project(hunnu_lang VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/compiler)

set(HUNNU_SOURCES
    cli/main.c
    cli/cli.c
    compiler/lexer/lexer.c
    compiler/lexer/token.c
    compiler/parser/parser.c
    compiler/ast/ast.c
    compiler/interpreter/interpreter.c
)

add_executable(hunnu ${HUNNU_SOURCES})

target_compile_options(hunnu PRIVATE -Wall -Wextra -pedantic)

install(TARGETS hunnu DESTINATION bin)