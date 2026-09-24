#include "rush/terminal.hpp"

#include "rush/io.hpp"

#include <format>
#include <string>
#include <termios.h>
#include <unistd.h>

namespace {

constexpr char ESC = 0x1B;
constexpr char ENTER = 0x0D;
constexpr char TAB = 0x09;
constexpr char BACKSPACE_H = 0x08;
constexpr char DEL = 0x7F;

} // namespace

Terminal::Terminal(const std::string &s) : pwd(s) {
    tcgetattr(STDIN_FILENO, &original);
    raw = original;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
}

std::string Terminal::prompt() { return std::format("{}$ ", pwd); }

void Terminal::enter_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); }
void Terminal::exit_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

Key Terminal::read_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return {KeyType::Unknown};

    if (c == ESC) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return {KeyType::Unknown};
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return {KeyType::Unknown};
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return {KeyType::Unknown};
                if (seq[2] == '~') {
                    switch (seq[1]) {
                    case '3':
                        return {KeyType::Delete};
                    case '5':
                        return {KeyType::PageUp};
                    case '6':
                        return {KeyType::PageDown};
                    }
                }
            } else {
                switch (seq[1]) {
                case 'A':
                    return {KeyType::ArrowUp};
                case 'B':
                    return {KeyType::ArrowDown};
                case 'C':
                    return {KeyType::ArrowRight};
                case 'D':
                    return {KeyType::ArrowLeft};
                case 'H':
                    return {KeyType::Home};
                case 'F':
                    return {KeyType::End};
                }
            }
        }
        return {KeyType::Unknown};
    }

    if (c == ENTER) return {KeyType::Enter};
    if (c == TAB) return {KeyType::Tab};
    if (c == BACKSPACE_H || c == DEL) return {KeyType::Backspace};
    if (c < 0x20) return {KeyType::Ctrl, static_cast<char>(c + 'A' - 1)};
    return {KeyType::Char, c};
}

void Terminal::draw(std::string text, size_t cursor) {
    if (cursor_row > 0) {
        write_str(up(cursor_row));
    }

    // clear till end of screen
    write_str("\r\x1b[J");

    write_str(prompt());
    write_str(text);

    size_t back = text.size() - cursor;
    if (back) write_str(left(back));
}

void Terminal::new_line() { write_str("\n\r"); }
