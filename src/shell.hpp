#pragma once

#include <string>
#include <vector>

#define PROMPT "> "
#define QUOTED_PROMPT " "

enum class TokenizeStatus { UnfinishedQuote, Ok };
struct TokenizeResult {
    std::vector<std::string> tokens;
    std::string partial;
    char quote;
    TokenizeStatus status;
};

TokenizeResult tokenize(
    std::string input, std::string partial = "", char quote = '\0'
);
void launch_program(std::vector<std::string> &cmd);
