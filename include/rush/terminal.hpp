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

class Terminal {
  private:
    termios original;
    termios raw;

  public:
    Terminal();
    void enter_raw_mode();
    void exit_raw_mode();
    Key read_key();
    void draw(std::string text = "", size_t cursor = 0);
    void new_line();
};
