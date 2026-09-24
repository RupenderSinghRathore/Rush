#pragma once

#include <cstddef>
#include <string>
#include <termios.h>

enum class KeyType {
    Char,
    Enter,
    Backspace,
    Tab,
    Escape,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    PageUp,
    PageDown,
    Home,
    End,
    Delete,
    Ctrl,
    Unknown,
};

struct Key {
    KeyType type;
    char ch = '\0';
};

#define PROMPT "> "

class Terminal {
  private:
    termios original;
    termios raw;
    size_t cursor_row = 0;
    const std::string &pwd;

  public:
    Terminal(const std::string &s);
    void enter_raw_mode();
    void exit_raw_mode();
    Key read_key();
    void draw(std::string text = "", size_t cursor = 0);
    void new_line();

    std::string up(int i) { return "\x1b[" + std::to_string(i) + "A"; }
    std::string down(int i) { return "\x1b[" + std::to_string(i) + "B"; }
    std::string right(int i) { return "\x1b[" + std::to_string(i) + "C"; }
    std::string left(int i) { return "\x1b[" + std::to_string(i) + "D"; }
    std::string prompt();
    void increment_row() { cursor_row++; };
    void reset_row() { cursor_row = 0; };
};
