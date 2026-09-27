#include "rush/tokenizer.hpp"

#include <cctype>
#include <string>
#include <string_view>
#include <utility>

void Tokenizer::tokenize(std::string line) {
    for (auto c : line) {
        consume_char(c);
    }
}

TokenType Tokenizer::check_operator() {
    for (const auto &[text, type] : operators) {
        if (token.text == text) {
            token.type = type;
            return type;
        }
    }
    return TokenType::Unknown;
}

void Tokenizer::consume_char(char c) {
    if (quote) {
        if (c == quote)
            quote = '\0';
        else
            token.text.push_back(c);
        return;
    }
    if (std::isspace(static_cast<unsigned char>(c))) {
        if (started) {
            if (check_operator() == TokenType::Unknown)
                token.type = TokenType::Word;

            tokens.push_back(std::move(token));
            token.text.clear();
            started = false;
            token.type = TokenType::Unknown;
        }
        return;
    }
    if (c == '\'' || c == '"') {
        quote = c;
        started = true;
        return;
    }

    token.text.push_back(c);
    started = true;
}

void Tokenizer::clear() {
    tokens.clear();
    token = {};
    quote = '\0';
    started = false;
}

bool Tokenizer::is_identifier_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_' ||
           c == '.' || c == '/';
}

bool Tokenizer::is_identifier(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_' ||
           c == '.' || c == '/';
}

bool Tokenizer::is_quoted() { return quote; }

bool Tokenizer::is_empty() { return tokens.empty(); }

std::vector<Token> Tokenizer::owned_tokens() { return std::move(tokens); }
