#pragma once

#include <format>
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
    Token token;
    char quote = '\0';
    bool started = false;

    Tokenizer() = default;

    void consume_char(char c);
    bool finish_tokenizer();
};

class Shell {
  private:
  public:
    Terminal term;
    Tokenizer tokenizer;

    Shell() = default;

    void repl();
    void execute_command();
    void write_char(char c);
    void write_str(const std::string &s);

    template <typename... Args>
    void write_err(std::format_string<Args...> fmt, Args &&...args);
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
