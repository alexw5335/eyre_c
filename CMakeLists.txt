cmake_minimum_required(VERSION 3.24)
project(eyre_c C)
set(CMAKE_C_STANDARD 23)



add_executable(
        eyre
        src/main.c
        src/internal.c
        src/internal.h
        src/enums.h
        src/enums.c
        src/encodings.c
        src/encodings.h
        src/nodes.h
        src/gen.c
        src/gen.h
        src/symbols.h
        src/intern.h
        src/intern.c
        src/lexer.c
        src/parser.c
        src/resolve.c
        src/assembler.c
        src/linker.c
        src/buffer.h)