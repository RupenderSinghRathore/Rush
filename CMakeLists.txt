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

# list test files
set(TEST_SOURCES
    test/main.cpp
    test/test_tokenizer.cpp
)

# Build unified test runner executable
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE rush_lib)
target_include_directories(unit_tests PRIVATE include)

# Register a single test with CTest
add_test(NAME AllUnitTests COMMAND unit_tests)
