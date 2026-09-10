#include "shell.hpp"
#include <cctype>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <print>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

int main() {
    std::string buf;
    while (1) {
        std::cout << PROMPT;

        std::getline(std::cin, buf);
        auto result = tokenize(buf);

        while (result.status == TokenizeStatus::UnfinishedQuote) {
            std::cout << QUOTED_PROMPT;
            std::getline(std::cin, buf);
            auto other = tokenize(buf, result.partial, result.quote);
            result.tokens.insert(
                result.tokens.end(),
                std::make_move_iterator(other.tokens.begin()),
                std::make_move_iterator(other.tokens.end())
            );
            result.status = other.status;
        }

        if (result.tokens.empty()) continue;

        // std::println("tokens: {}", result.tokens);
        launch_program(result.tokens);
    }
    return 0;
}

TokenizeResult tokenize(std::string input, std::string partial, char quote) {
    std::vector<std::string> tokens;
    std::string token = partial;

    bool started = !partial.empty();

    for (auto c : input) {
        if (quote) {
            if (c == quote)
                quote = '\0';
            else
                token.push_back(c);
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (started) {
                tokens.push_back(std::move(token));
                token.clear();
                started = false;
            }
            continue;
        }
        if (c == '\'' || c == '"') {
            quote = c;
            started = true;
            continue;
        }

        token.push_back(c);
        started = true;
    }
    if (quote) {
        return {
            std::move(tokens), token, quote, TokenizeStatus::UnfinishedQuote
        };
    }
    if (started) {
        tokens.push_back(std::move(token));
    }

    return {std::move(tokens), "", quote, TokenizeStatus::Ok};
}

void launch_program(std::vector<std::string> &tokens) {
    auto pid = fork();
    if (pid == 0) {
        std::vector<char *> cmd;
        for (auto &s : tokens) {
            cmd.push_back(s.data());
        }
        cmd.push_back(nullptr);
        execvp(cmd[0], cmd.data());
        perror("error in resonance");
        _exit(127);
    } else if (pid < 0) {
        perror("error in fork");
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
}
