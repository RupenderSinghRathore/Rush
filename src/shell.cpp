#include "rush.hpp"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <print>
#include <unistd.h>

int main() {
    auto shell = Shell();
    shell.term.enter_raw_mode();
    shell.repl();
    shell.term.exit_raw_mode();
    return 0;
}

// extract lower 5 bits
#define CTRL_KEY(k) ((k) & 0x1f)

void Shell::repl() {
    char c;
    while (1) {
        int r = read(STDIN_FILENO, &c, 1);
        assert(r == 1);

        if (!std::iscntrl(c)) {
            tokenizer.consume_char(c);
            std::putchar(c);
            std::fflush(stdout);
        }
        if (c == CTRL_KEY('Q')) {
            break;
        }
        if (c == '\x1b') {
            // Potentially Alt or an escape sequence
        }
    }
}


void Tokenizer::consume_char(char c) {
}
