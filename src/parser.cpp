#include "rush/parser.hpp"

#include <stdexcept>
#include <utility>

void Parser::reset(std::vector<Token> t) {
    tokens = std::move(t);
    pos = 0;
}

Expr Parser::parse_start() {
    auto expr = parse_expr();
    if (peek()) {
        throw std::runtime_error("unexpected trailing tokens: " + peek()->text);
    }
    return expr;
}

const Token *Parser::peek() {
    if (pos < tokens.size()) {
        return &tokens[pos];
    }
    return nullptr;
}

Token Parser::advance() {
    if (pos >= tokens.size()) {
        throw std::runtime_error("unexpected end of input");
    }
    return tokens[pos++];
}

std::string Parser::expect_identifier() {
    Token t = advance();
    if (t.type != TokenType::Identifier) {
        throw std::runtime_error("expected identifier, got: " + t.text);
    }
    return t.text;
}

std::string Parser::expect_constant() {
    Token t = advance();
    if (t.type != TokenType::Constant) {
        throw std::runtime_error("expected Constant, got: " + t.text);
    }
    return t.text;
}

std::string Parser::expect_infix_operator() {
    Token t = advance();
    if (t.type != TokenType::Infix_Operator) {
        throw std::runtime_error("expected Operator, got: " + t.text);
    }
    return t.text;
}

std::string Parser::expect_postfix_operator() {
    Token t = advance();
    if (t.type != TokenType::Postfix_Operator) {
        throw std::runtime_error("expected Operator, got: " + t.text);
    }
    return t.text;
}

Expr Parser::parse_expr() {
    auto left = parse_term();
    std::vector<std::pair<std::string, Term>> rest;
    while (peek() && peek()->type == TokenType::Infix_Operator) {
        auto op = expect_infix_operator();
        auto term = parse_term();
        rest.push_back({std::move(op), std::move(term)});
    }
    return Expr(std::move(left), std::move(rest));
}

Term Parser::parse_term() {
    auto term = Term{};
    term.name = expect_identifier();
    term.args = parse_args();
    if (peek() && peek()->type == TokenType::Postfix_Operator) {
        term.postfix_op = expect_postfix_operator();
    }
    return term;
}

std::vector<std::string> Parser::parse_args() {
    std::vector<std::string> args;
    while (peek() && peek()->type == TokenType::Constant) {
        args.push_back(expect_constant());
    }
    return args;
}
