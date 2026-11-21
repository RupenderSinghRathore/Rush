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
    struct test_case {
        std::string input;
        std::vector<Token> expect;
    };

    std::vector<test_case> tests = {
        {
            .input = "ls -la ",
            .expect =
                std::vector<Token>{
                    Token{TokenType::Word, "ls"}, Token{TokenType::Word, "-la"}
                },
        },
        {
            .input = "ls -la",
            .expect = std::vector<Token>{Token{TokenType::Word, "ls"}},
        }
    };

    for (auto &tc : tests) {
        t.tokenize(tc.input);
        auto got = t.owned_tokens();

        ASSERT_EQ(got, tc.expect);

        t.clear();
    }
}
