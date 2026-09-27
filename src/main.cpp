#include "rush/shell.hpp"

int main() {
    auto shell = Shell();
    return shell.repl();
}
