cmake_minimum_required(VERSION 3.24)
project(eyre_c C)

set(CMAKE_C_STANDARD 23)

add_executable(
        eyre
        src/main.c
        src/eyre.c
        src/eyre.h
        src/intern.c
        src/lexer.c
        src/parser.c
        src/log.h
        src/log.c
        src/lex.h
)

target_compile_definitions(
        eyre
        PUBLIC
        EYRE_LOG_ALL
)

add_executable(
        gen
        gen/generator.c
)