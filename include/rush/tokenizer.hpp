#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

enum class TokenType {
    Unknown,
    Word,
    LineEnd,
    Background,
    And,
    Or,
    Pipe,
    PipeStderr,
    OutputRedirect,
    OutputAppendRedirect,
    InputRedirect,
    HereDocument,
};

constexpr std::array<std::pair<std::string_view, TokenType>, 6> operators{{
    {";", TokenType::LineEnd},
    {"&", TokenType::Background},
    {"&&", TokenType::And},
    {"||", TokenType::Or},
    {"|", TokenType::Pipe},
    {"|&", TokenType::PipeStderr},
}};

struct Token {
    TokenType type;
    std::string text;
};

class Tokenizer {
  private:
    Token token;
    char quote = '\0';
    bool started = false;

    bool is_identifier_start(char);
    bool is_identifier(char);
    TokenType check_operator();

  public:
    std::vector<Token> tokens;

    Tokenizer() = default;
    void tokenize(std::string);
    void consume_char(char);
    void clear();
    bool is_quoted();
    bool is_empty();
    std::vector<Token> owned_tokens();
};
