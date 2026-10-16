#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

enum class TokenType {
    Unknown,
    Word,
    Semicolon,
    Ampersand,
    And,
    Or,
    Pipe,
    ErrPipe,
    HereDocument,
};

constexpr std::array<std::pair<std::string_view, TokenType>, 6> operators{{
    {";", TokenType::Semicolon},
    {"&", TokenType::Ampersand},
    {"&&", TokenType::And},
    {"||", TokenType::Or},
    {"|", TokenType::Pipe},
    {"|&", TokenType::ErrPipe},
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
