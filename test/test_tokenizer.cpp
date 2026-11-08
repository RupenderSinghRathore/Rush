#include "rush/tokenizer.hpp"
#include "test_framework.hpp"
#include <cassert>
#include <ostream>
#include <vector>

inline std::string to_string(TokenType type) {
    switch (type) {
    case TokenType::Unknown:
        return "Unknown";
    case TokenType::Word:
        return "Word";
    case TokenType::Semicolon:
        return "Semicolon";
    case TokenType::Ampersand:
        return "Ampersand";
    case TokenType::And:
        return "And";
    case TokenType::Or:
        return "Or";
    case TokenType::Pipe:
        return "Pipe";
    case TokenType::ErrPipe:
        return "ErrPipe";
    case TokenType::HereDocument:
        return "HereDocument";
    }
    return "Unknown";
}

inline std::ostream &operator<<(std::ostream &os, const Token &t) {
    return os << "Token{type: " << to_string(t.type) << ", val: \"" << t.text
              << "\"}";
}

TEST(test_tokenizer_tokenize) {
    Tokenizer t{};
    auto input = "ls -la ";
    auto expect_tokens = std::vector<Token>{
        Token{TokenType::Word, "ls"}, Token{TokenType::Word, "-la"}
    };
    t.tokenize(input);
    ASSERT_EQ(t.tokens, expect_tokens);
}
