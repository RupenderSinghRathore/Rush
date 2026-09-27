#pragma once

#include "rush/tokenizer.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <variant>
#include <vector>

/* GRAMMAR

Terminals (TokenTypes)

Start -> List EOF

List -> AndOr ListTail?

ListTail -> ';' ListTailInner?

ListTailInner -> List | ListTail

AndOr -> Pipeline AndOrTail?

AndOrTail -> '&&' Pipeline AndOrTail?
          | '||' Pipeline AndOrTail?

Pipeline -> Background PipelineTail?

PipelineTail -> '|' Background PipelineTail?
             | '|&' Background PipelineTail?
             | '>' Background PipelineTail?
             | '>>' Background PipelineTail?
             | '<' Background PipelineTail?
             | '<<' Background PipelineTail?

Background -> Command BackgroundTail?

BackgroundTail -> '&' list?

Command -> Word+

*/

struct List;
struct ListTail;
struct AndOr;
struct AndOrTail;
struct Pipeline;
struct PipelineTail;
struct Background;
struct BackgroundTail;

using ListTailInner =
    std::variant<std::unique_ptr<List>, std::unique_ptr<ListTail>>;

struct List {
    std::unique_ptr<AndOr> and_or;
    std::unique_ptr<ListTail> tail; // optional
};

struct ListTail {
    std::string op;                       // ;
    std::unique_ptr<ListTailInner> inner; // optional
};

struct AndOr {
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<AndOrTail> and_or_tail; // optional
};

struct AndOrTail {
    std::string op; // && ||
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<AndOrTail> and_or_tail; // optional
};

struct Pipeline {
    std::unique_ptr<Background> bg;
    std::unique_ptr<PipelineTail> pipeline_tail; // optional
};

struct PipelineTail {
    std::string op; // | |& > >> << <
    std::unique_ptr<Background> bg;
    std::unique_ptr<PipelineTail> pipeline_tail; // optional
};

struct Background {
    std::vector<std::string> cmd;
    std::unique_ptr<BackgroundTail> bg_tail; // optional
};

struct BackgroundTail {
    std::string op;             // &
    std::unique_ptr<List> list; // optional
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
    std::unique_ptr<ListTailInner> parse_list_tail_inner();
    std::unique_ptr<AndOr> parse_and_or();
    std::unique_ptr<AndOrTail> parse_and_or_tail();
    std::unique_ptr<Pipeline> parse_pipeline();
    std::unique_ptr<PipelineTail> parse_pipeline_tail();
    std::unique_ptr<Background> parse_background();
    std::unique_ptr<BackgroundTail> parse_background_tail();

    std::vector<std::string> parse_cmd();
};
