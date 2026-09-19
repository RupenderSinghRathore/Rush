#pragma once

#include "rush/tokenizer.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

/* GRAMMAR

Terminals (TokenTypes)

Start -> Expr
Expr -> Term (Infix_Operator Term)*
Term -> Identifier Args Postfix_Operator?
Args -> Constant*

*/

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
