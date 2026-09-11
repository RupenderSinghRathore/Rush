#include "rush.hpp"
#include <termios.h>
#include <unistd.h>

Terminal::Terminal() {
    tcgetattr(STDIN_FILENO, &original);
    raw = original;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
}

void Terminal::enter_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); }
void Terminal::exit_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}
