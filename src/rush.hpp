#pragma once

#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define PROMPT "> "
#define QUOTED_PROMPT " "

class Terminal {
  private:
    termios original;
    termios raw;

  public:
    Terminal();
    void enter_raw_mode();
    void exit_raw_mode();
};

enum class TokenType { Identifier, Constant };
struct Token {
    TokenType type;
    std::string text;
};
class Tokenizer {
  public:
    std::vector<Token> tokens;
    char quote;
    bool started;
    Token token;

    Tokenizer() = default;

    void consume_char(char c);
};

class Shell {
  private:
  public:
    Terminal term;
    Tokenizer tokenizer;

    Shell() = default;

    void repl();
};

/* GRAMMER
Identifier -> Const*
*/

enum class TokenizeStatus { UnfinishedQuote, Ok };
struct TokenizeResult {
    std::vector<std::string> tokens;
    std::string partial;
    char quote;
    TokenizeStatus status;
};

TokenizeResult tokenize(
    std::string input, std::string partial = "", char quote = '\0'
);
void launch_program(std::vector<std::string> &cmd);
