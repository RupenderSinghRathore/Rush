#include "rush/parser.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

void Parser::reset(std::vector<Token> t) {
    tokens = std::move(t);
    pos = 0;
}

std::unique_ptr<List> Parser::parse_start() {
    auto list = parse_list();
    if (peek()) {
        throw std::runtime_error("unexpected trailing tokens: " + peek()->text);
    }
    return list;
}

const Token *Parser::peek() {
    if (pos < tokens.size()) {
        return &tokens[pos];
    }
    return nullptr;
}

Token &Parser::advance() {
    if (pos >= tokens.size()) {
        throw std::runtime_error("unexpected end of input");
    }
    return tokens[pos++];
}

std::string Parser::expect_word() {
    auto &t = advance();
    if (t.type != TokenType::Word) {
        throw std::runtime_error("expected Word, got: " + t.text);
    }
    return t.text;
}

std::string Parser::expect_operator() {
    auto &t = advance();
    for (const auto [_, op] : operators) {
        if (t.type == op) return t.text;
    }
    throw std::runtime_error("expected Operator, got: " + t.text);
}

std::unique_ptr<List> Parser::parse_list() {
    auto and_or = parse_and_or();

    auto next = peek();
    if (next && (next->type == TokenType::LineEnd ||
                 next->type == TokenType::Background)) {
        return std::make_unique<List>(std::move(and_or), parse_list_tail());
    }

    if (next) {
        throw std::runtime_error("expected end of input, got: " + next->text);
    }

    return std::make_unique<List>(std::move(and_or), nullptr);
}

std::unique_ptr<ListTail> Parser::parse_list_tail() {
    auto &t = advance();
    if (t.type != TokenType::LineEnd && t.type != TokenType::Background) {
        throw std::runtime_error("expected Operator, got: " + t.text);
    }

    if (peek()) return std::make_unique<ListTail>(t.text, parse_list());

    return std::make_unique<ListTail>(t.text, nullptr);
}

std::unique_ptr<AndOr> Parser::parse_and_or() {
    auto pipeline = parse_pipeline();

    auto next = peek();
    if (next && (next->type == TokenType::And || next->type == TokenType::Or)) {
        return std::make_unique<AndOr>(
            std::move(pipeline), parse_and_or_tail()
        );
    }

    return std::make_unique<AndOr>(std::move(pipeline), nullptr);
}
std::unique_ptr<AndOrTail> Parser::parse_and_or_tail() {
    auto &t = advance();
    if (t.type != TokenType::And && t.type != TokenType::Or) {
        throw std::runtime_error("expected Operator, got: " + t.text);
    }

    auto pipeline = parse_pipeline();

    auto next = peek();
    if (next && (next->type == TokenType::And || next->type == TokenType::Or)) {
        return std::make_unique<AndOrTail>(
            t.text, std::move(pipeline), parse_and_or_tail()
        );
    }

    return std::make_unique<AndOrTail>(t.text, std::move(pipeline), nullptr);
}
std::unique_ptr<Pipeline> Parser::parse_pipeline() {
    auto cmd = parse_cmd();

    auto next = peek();
    if (next && (next->type == TokenType::Pipe ||
                 next->type == TokenType::PipeStderr)) {
        return std::make_unique<Pipeline>(
            std::move(cmd), parse_pipeline_tail()
        );
    }
    return std::make_unique<Pipeline>(std::move(cmd), nullptr);
}
std::unique_ptr<PipelineTail> Parser::parse_pipeline_tail() {
    auto &t = advance();
    if (t.type != TokenType::Pipe && t.type != TokenType::PipeStderr) {
        throw std::runtime_error("expected Operator, got: " + t.text);
    }

    auto cmd = parse_cmd();

    auto next = peek();
    if (next && (next->type == TokenType::Pipe ||
                 next->type == TokenType::PipeStderr)) {
        return std::make_unique<PipelineTail>(
            t.text, std::move(cmd), parse_pipeline_tail()
        );
    }

    return std::make_unique<PipelineTail>(t.text, std::move(cmd), nullptr);
}

std::vector<std::string> Parser::parse_cmd() {
    std::vector<std::string> cmd;
    do {
        cmd.push_back(expect_word());
    } while (peek() && peek()->type == TokenType::Word);
    return cmd;
}
