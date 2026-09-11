#include "rush.hpp"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <print>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

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
            }

            if (tokenizer.tokens.empty()) {
                write_str(PROMPT);
                continue;
            }

            // TODO: Parser

            term.exit_raw_mode();
            execute_command();
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
bool Tokenizer::finish_tokenizer() {
    if (quote) {
        return false;
    }
    if (started) {
        token.type =
            tokens.empty() ? TokenType::Identifier : TokenType::Constant;
        tokens.push_back(std::move(token));
        token.text.clear();
    }
    return true;
}

void Shell::execute_command() {
    auto pid = fork();
    if (pid == 0) {
        std::vector<char *> cmd;
        for (auto &t : tokenizer.tokens) {
            cmd.push_back(t.text.data());
        }
        cmd.push_back(nullptr);
        execvp(cmd[0], cmd.data());
        write_err("unrecognized command: {}\n", cmd[0]);
        _exit(127);
    } else if (pid < 0) {
        perror("error in fork");
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
}
