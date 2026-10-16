#pragma once

#include "rush/tokenizer.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/* GRAMMAR

Terminals (TokenTypes)

Start -> List EOF

List -> AndOr (ListMid)* ListSeparator?
ListMid -> ListSeparator AndOr
ListSeparator -> ';' | '&'

AndOr -> Pipeline (AndOrMid)*
AndOrMid -> AndOrOperator Pipeline
AndOrOperator -> '&&' | '||'

Pipeline -> Command (PipelineMid)*
PipelineMid -> PipelineOperator Command
PipelineOperator -> '|' | '|&'

Command -> Word+

*/

struct List;
struct ListMid;
enum class ListSeparator { Semicolon, Ampersand };

struct AndOr;
struct AndOrMid;
enum class AndOrOperator { And, Or };

struct Pipeline;
struct PipelineMid;
enum class PipelineOperator { Pipe, ErrPipe };

struct List {
    std::unique_ptr<AndOr> and_or;
    std::vector<ListMid> mid;
    std::optional<ListSeparator> sep;
};

struct ListMid {
    ListSeparator sep;
    std::unique_ptr<AndOr> and_or;
};

struct AndOr {
    std::unique_ptr<Pipeline> pipeline;
    std::vector<AndOrMid> mid;
};

struct AndOrMid {
    AndOrOperator op;
    std::unique_ptr<Pipeline> pipeline;
};

struct Pipeline {
    std::vector<std::string> cmd;
    std::vector<PipelineMid> mid;
};

struct PipelineMid {
    PipelineOperator op;
    std::vector<std::string> cmd;
};

class Parser {
  public:
    std::vector<Token> tokens;
    size_t pos;

    Parser() = default;
    void reset(std::vector<Token>);
    std::unique_ptr<List> parse_start();
    const Token *peek();
    const Token *peek_next();
    Token &advance();

    bool is_list_separator(TokenType t) {
        return t == TokenType::Semicolon || t == TokenType::Ampersand;
    }
    bool is_and_or_operator(TokenType t) {
        return t == TokenType::And || t == TokenType::Or;
    }
    bool is_pipeline_operator(TokenType t) {
        return t == TokenType::Pipe || t == TokenType::ErrPipe;
    }

    std::string expect_word();
    std::string expect_operator();

    std::unique_ptr<List> parse_list();
    ListMid parse_list_mid();
    ListSeparator parse_list_separator();

    std::unique_ptr<AndOr> parse_and_or();
    AndOrMid parse_and_or_mid();
    AndOrOperator parse_and_or_operator();

    std::unique_ptr<Pipeline> parse_pipeline();
    PipelineMid parse_pipeline_mid();
    PipelineOperator parse_pipeline_operator();

    std::vector<std::string> parse_cmd();
};
