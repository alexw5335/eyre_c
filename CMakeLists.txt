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
        src/parser.c src/log.h src/log.c)

# Defines:
# EYRE_LOG_TRACE: Log trace-level logs
# EYRE_LOG_INFO:  Log info-level logs
# EYRE_LOG_DEBUG: Log debug-level logs
# EYRE_LOG_ALL:   Log everything
target_compile_definitions(
        eyre
        PUBLIC
        EYRE_LOG_ALL
)
