cmake_minimum_required(VERSION 3.24)
project(eyre_c C)

set(CMAKE_C_STANDARD 23)

add_executable(
        eyre
        src/main.c
        src/eyre.c
        src/eyre.h
        src/intern.c
        src/lex.c
        src/parse.c
        src/log.h
        src/log.c
        src/lex.h
        src/symbol.c
        src/defs.h
        src/encodings.c
        src/gen.c
        src/gen.h
        src/defs.c
        src/intern.h
        src/parse.h
        src/symbol.h
        src/resolve.c
        src/resolve.h
        src/assemble.c src/assemble.h src/link.c src/link.h src/internal.c src/internal.h)

target_compile_definitions(
        eyre
        PUBLIC
        EYRE_LOG_ALL
)