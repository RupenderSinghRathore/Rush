#include "rush/tokenizer.hpp"
#include <doctest/doctest.h>
#include <ostream>
#include <string>
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

TEST_CASE("test_tokenizer_tokenize") {
    Tokenizer t{};
    struct test_case {
        std::string name;
        std::string input;
        std::vector<Token> expect;
    };

    std::vector<test_case> tests = {
        {
            .name = "delimited input",
            .input = "ls -la\n",
            .expect =
                std::vector<Token>{
                    Token{TokenType::Word, "ls"}, Token{TokenType::Word, "-la"}
                },
        },
        {
            .name = "undelimited input",
            .input = "ls -la",
            .expect = std::vector<Token>{Token{TokenType::Word, "ls"}},
        }
    };

    for (auto &tc : tests) {
        SUBCASE(tc.name) {
            t.tokenize(tc.input);
            auto got = t.owned_tokens();

            CHECK(got == tc.expect);

            t.clear();
        }
    }
}

TEST_CASE("test_tokenizer_clear") {

    struct test_case {
        std::string name;
        std::string input;
        bool empty_before_clear;
        bool empty_after_clear;
    };

    std::vector<test_case> tests = {
        {
            .name = "delimited input",
            .input = "ls -la\n",
            .empty_before_clear = false,
            .empty_after_clear = true,
        },
        {
            .name = "single undelimited input",
            .input = "ls",
            .empty_before_clear = true,
            .empty_after_clear = true,
        },
    };

    auto t = Tokenizer{};

    for (auto &tc : tests) {
        SUBCASE(tc.name) {
            t.tokenize(tc.input);

            CHECK(!t.has_processed_tokens() == tc.empty_before_clear);

            t.clear();

            CHECK(!t.has_processed_tokens() == tc.empty_after_clear);
        }
    }
}

TEST_CASE("test_tokenizer_owned_tokens") {
    struct test_case {
        std::string name;
        std::string input;
        bool has_processed_tokens_before;
        bool has_processed_tokens_after;
    };

    std::vector<test_case> tests = {
        {
            .name = "delimited input",
            .input = "ls -la\n",
            .has_processed_tokens_before = true,
            .has_processed_tokens_after = false,
        },
        {
            .name = "single undelimited input",
            .input = "ls",
            .has_processed_tokens_before = false,
            .has_processed_tokens_after = false,
        },
    };

    auto t = Tokenizer{};

    for (auto &tc : tests) {
        SUBCASE(tc.name) {
            t.tokenize(tc.input);

            CHECK(t.has_processed_tokens() == tc.has_processed_tokens_before);

            t.owned_tokens();

            CHECK(t.has_processed_tokens() == tc.has_processed_tokens_after);
        }
    }
}

TEST_CASE("test_tokenizer_is_quoted") {
    struct test_case {
        std::string name;
        std::string input;
        bool quoted;
    };

    std::vector<test_case> tests = {
        {
            .name = "unquoted",
            .input = "ls -la\n",
            .quoted = false,
        },
        {
            .name = "unfinished quote",
            .input = "echo 'hell",
            .quoted = true,
        },
        {
            .name = "finished quote",
            .input = "echo 'hello world'",
            .quoted = false,
        },
    };

    auto t = Tokenizer{};

    for (auto &tc : tests) {
        SUBCASE(tc.name) {
            t.tokenize(tc.input);

            CHECK(t.is_quoted() == tc.quoted);

            t.clear();
        }
    }
}

TEST_CASE("test_tokenizer_has_processed_tokens") {
    struct test_case {
        std::string name;
        std::string input;
        bool processed_tokens;
    };

    std::vector<test_case> tests = {
        {
            .name = "multi-word delimited input",
            .input = "ls -la\n",
            .processed_tokens = true,
        },
        {
            .name = "multi-word non-delimited input",
            .input = "ls -la",
            .processed_tokens = true,
        },
        {
            .name = "single non delimited input",
            .input = "ls",
            .processed_tokens = false,
        },
        {
            .name = "single delimited input",
            .input = "ls\n",
            .processed_tokens = true,
        },
    };

    auto t = Tokenizer{};

    for (auto &tc : tests) {
        SUBCASE(tc.name) {
            t.tokenize(tc.input);

            CHECK(t.has_processed_tokens() == tc.processed_tokens);

            t.clear();
        }
    }
}
