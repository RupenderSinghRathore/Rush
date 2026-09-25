#pragma once

#include "rush/tokenizer.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* GRAMMAR

Terminals (TokenTypes)

Start -> Expr
Expr -> Term (Infix_Operator Term)*
Term -> Identifier Args Postfix_Operator?
Args -> Constant*


Start → List EOF

List → AndOr ListTail?

ListTail → ';' List?
         | '&' List?

AndOr → Pipeline AndOrTail?

AndOrTail → '&&' Pipeline AndOrTail
          | '||' Pipeline AndOrTail

Pipeline → Command PipelineTail?

PipelineTail → '|' Command PipelineTail
             | '|&' Command PipelineTail

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
    std::unique_ptr<ListTail> tail;
};

struct ListTail {
    std::string op;
    std::unique_ptr<List> list;
};

struct AndOr {
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<AndOrTail> and_or_tail;
};

struct AndOrTail {
    std::string op;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<AndOrTail> and_or_tail;
};

struct Pipeline {
    std::vector<std::string> cmd;
    std::unique_ptr<PipelineTail> pipeline_tail;
};

struct PipelineTail {
    std::string op;
    std::vector<std::string> cmd;
    std::unique_ptr<PipelineTail> pipeline_tail;
};

struct Term {
    std::string name;
    std::vector<std::string> args;
    std::string postfix_op;
};

struct Expr {
    Term left;
    std::vector<std::pair<std::string, Term>> rest;
};

class Parser {
  public:
    std::vector<Token> tokens;
    size_t pos;

    Parser() = default;
    void reset(std::vector<Token> t);
    Expr parse_start();
    const Token *peek();
    Token advance();
    std::string expect_identifier();
    std::string expect_constant();
    std::string expect_infix_operator();
    std::string expect_postfix_operator();
    Expr parse_expr();
    Term parse_term();
    std::vector<std::string> parse_args();
};
