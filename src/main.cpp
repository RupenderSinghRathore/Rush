#include "shell.hpp"
#include <cctype>
#include <cstddef>
#include <iostream>
#include <print>
#include <string>
#include <vector>

int main() {
    while (1) {
        std::printf("> ");
        std::string input;
        std::getline(std::cin, input);

        // input.push_back('\n');
        auto tokens = tokanize(input);

        std::println("tokens: {}", tokens);
    }
    return 0;
}

std::vector<std::string> tokanize(std::string input) {
    std::vector<std::string> tokens;
    char quote = '\0';
    std::string token;
    bool token_started = false;
    do {
        for (size_t i = 0; i < input.size(); i++) {
            char c = input[i];
            if (std::isspace(c)) {
                if (!quote) {
                    if (token_started) {
                        tokens.push_back(token);
                        token.clear();
                        token_started = false;
                    }
                } else {
                    token.push_back(c);
                }
                continue;
            }

            if (input[i] == '\'' or c == '"') {
                if (!quote) {
                    quote = c;
                    token_started = true;
                    continue;
                }
                if (quote == c) {
                    quote = '\0';
                    continue;
                }
            }

            token.push_back(c);
            token_started = true;
        }
        if (!quote && token_started) {
            tokens.push_back(token);
            token.clear();
        }
        if (quote) {
            std::print(" ");
            std::getline(std::cin, input);
        }
    } while (quote);
    return tokens;
}
