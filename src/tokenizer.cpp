#include "rush/tokenizer.hpp"

#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

void Tokenizer::tokenize(std::string line) {
    for (auto c : line) {
        consume_char(c);
    }
}

// || & && ; ;; ;& ;;& ( ) | |& <newline>
constexpr std::string_view BACKGROUND_POSTFIX_OP = "&";

TokenType Tokenizer::check_operator() {
    if (token.text == BACKGROUND_POSTFIX_OP) return TokenType::Postfix_Operator;
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
            if (token.type == TokenType::Unknown) {
                token.type = check_operator();
                if (token.type == TokenType::Unknown)
                    token.type = TokenType::Constant;
            }

            tokens.push_back(std::move(token));
            token.text.clear();
            started = false;

            if (token.type == TokenType::Infix_Operator)
                token.type = TokenType::Identifier;
            else
                token.type = TokenType::Unknown;
        }
        return;
    }
    if (c == '\'' || c == '"') {
        quote = c;
        started = true;
        return;
    }

    if (token.type == TokenType::Identifier) {
        if (token.text.empty()) {
            if (is_identifier_start(c)) {
                token.text.push_back(c);
                started = true;
                return;
            }
        } else if (is_identifier(c)) {
            token.text.push_back(c);
            return;
        }
        throw std::runtime_error(
            "unexpected trailing character: " + std::string(1, c)
        );
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
