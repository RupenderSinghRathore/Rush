#pragma once

#include <string>
#include <vector>

enum class TokenType {
    Identifier = 0,
    Constant,
    Infix_Operator,
    Postfix_Operator,
    Unknown,
};

struct Token {
    TokenType type;
    std::string text;
};

class Tokenizer {
  private:
    Token token;
    char quote = '\0';
    bool started = false;

    bool is_identifier_start(char c);
    bool is_identifier(char c);

  public:
    std::vector<Token> tokens;

    Tokenizer() = default;
    void tokenize(std::string line);
    void consume_char(char c);
    void clear();
    bool is_quoted();
    bool is_empty();
    std::vector<Token> owned_tokens();
};
