#include "rush/io.hpp"

#include <unistd.h>

void write_char(char c) { write(STDOUT_FILENO, &c, 1); }

void write_str(const std::string &s) {
    write(STDOUT_FILENO, s.data(), s.size());
}

void write_err(const std::string &s) {
    write(STDERR_FILENO, s.data(), s.size());
}
