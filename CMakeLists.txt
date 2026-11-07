cmake_minimum_required(VERSION 4.4)
project(rush)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(run
    src/main.cpp
    src/shell.cpp
    src/editor.cpp
    src/evaluator.cpp
    src/io.cpp
    src/parser.cpp
    src/terminal.cpp
    src/tokenizer.cpp
)

target_include_directories(run PRIVATE
    include
)

target_compile_options(run PRIVATE
    $<$<CONFIG:Debug>:-g3 -O0 -Wall -Wextra -fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)
