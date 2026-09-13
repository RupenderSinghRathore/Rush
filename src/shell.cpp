#include "rush.hpp"
#include <cassert>
#include <cctype>
#include <print>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// extract lower 5 bits
#define CTRL_KEY(k) ((k) & 0x1f)

const char Enter = '\r';

void Shell::write_char(char c) { write(STDOUT_FILENO, &c, 1); }

void Shell::write_str(const std::string &s) {
    write(STDOUT_FILENO, s.data(), s.size());
}

template <typename... Args>
void Shell::write_err(std::format_string<Args...> fmt, Args &&...args) {
    std::print(stderr, fmt, std::forward<Args>(args)...);
}

void Shell::repl() {
    term.enter_raw_mode();

    char c;
    write_str(PROMPT);
    while (1) {
        int r = read(STDIN_FILENO, &c, 1);
        assert(r == 1);

        if (!std::iscntrl(c)) {
            tokenizer.consume_char(c);
            write_char(c);
        } else if (c == Enter) {
            write_str("\r\n");
            if (!tokenizer.finish_tokenizer()) {
                write_char(' ');
                continue;
            }

            parser.reset(std::move(tokenizer.tokens));
            auto ast = parser.parse_start();

            term.exit_raw_mode();
            process_expr(ast);
            term.enter_raw_mode();

            tokenizer.tokens.clear();
            write_str(PROMPT);
        }
        if (c == CTRL_KEY('Q')) {
            break;
        }
        if (c == '\x1b') {
            // Potentially Alt or an escape sequence
        }
    }
    term.exit_raw_mode();
}

bool is_identifier_start(char c) {
    return isalpha(c) || c == '_' || c == '.' || c == '/';
}
bool is_identifier(char c) {
    return isalnum(c) || c == '_' || c == '.' || c == '/';
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

            tokens.push_back(std::move(token));
            token.text.clear();
            started = false;

            if (token.type == TokenType::Identifier) {
                token.type = TokenType::Unknown;
            } else if (token.type == TokenType::Unknown) {
                // TODO: operator check

                token.type = TokenType::Constant;
            }
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
bool Tokenizer::finish_tokenizer() {
    if (quote) {
        return false;
    }
    if (started) {
        if (token.type == TokenType::Unknown) token.type = TokenType::Constant;
        tokens.push_back(std::move(token));
        started = false;
    }

    token = {};
    return true;
}
