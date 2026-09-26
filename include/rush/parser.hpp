#pragma once

#include "rush/tokenizer.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

/* GRAMMAR

Terminals (TokenTypes)

Start → List EOF

List → AndOr ListTail?

ListTail → ';' List?
         | '&' List?

AndOr → Pipeline AndOrTail?

AndOrTail → '&&' Pipeline AndOrTail?
          | '||' Pipeline AndOrTail?

Pipeline → Command PipelineTail?

PipelineTail → '|' Command PipelineTail?
             | '|&' Command PipelineTail?

Command → Word+

*/

struct List;
struct ListTail;
struct AndOr;
struct AndOrTail;
struct Pipeline;
struct PipelineTail;

struct List {
    std::unique_ptr<AndOr> and_or;
    std::unique_ptr<ListTail> tail; // optional
};

struct ListTail {
    std::string op;
    std::unique_ptr<List> list; // optional
};

struct AndOr {
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<AndOrTail> and_or_tail; // optional
};

struct AndOrTail {
    std::string op;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<AndOrTail> and_or_tail; // optional
};

struct Pipeline {
    std::vector<std::string> cmd;
    std::unique_ptr<PipelineTail> pipeline_tail; // optional
};

struct PipelineTail {
    std::string op;
    std::vector<std::string> cmd;
    std::unique_ptr<PipelineTail> pipeline_tail; // optional
};

class Parser {
  public:
    std::vector<Token> tokens;
    size_t pos;

    Parser() = default;
    void reset(std::vector<Token> t);
    std::unique_ptr<List> parse_start();
    const Token *peek();
    Token &advance();

    std::string expect_word();
    std::string expect_operator();

    std::unique_ptr<List> parse_list();
    std::unique_ptr<ListTail> parse_list_tail();
    std::unique_ptr<AndOr> parse_and_or();
    std::unique_ptr<AndOrTail> parse_and_or_tail();
    std::unique_ptr<Pipeline> parse_pipeline();
    std::unique_ptr<PipelineTail> parse_pipeline_tail();

    std::vector<std::string> parse_cmd();
};
