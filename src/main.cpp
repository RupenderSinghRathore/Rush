#include "rush.hpp"

int main() {
    auto shell = Shell();
    shell.term.enter_raw_mode();
    shell.repl();
    shell.term.exit_raw_mode();
    return 0;
}
