cmake_minimum_required(VERSION 3.20)
project(rush)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(rush_lib OBJECT
    src/shell.cpp
    src/editor.cpp
    src/evaluator.cpp
    src/io.cpp
    src/parser.cpp
    src/terminal.cpp
    src/tokenizer.cpp
)
target_include_directories(rush_lib PUBLIC include)

add_executable(rush src/main.cpp)
target_link_libraries(rush PRIVATE rush_lib)

enable_testing()

add_compile_options(
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-Wall>
    $<$<CONFIG:Debug>:-Wextra>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-DNDEBUG>
)

# Automatically find and configure all test files in the test/ directory
file(GLOB TEST_SOURCES test/test_*.cpp)

foreach(test_file ${TEST_SOURCES})
    # Extract filename without extension
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE rush_lib)
    target_include_directories(${test_name} PRIVATE include)

    # Register with CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
