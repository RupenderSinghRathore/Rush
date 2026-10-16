#include "rush/parser.hpp"
#include "rush/tokenizer.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

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

const Token *Parser::peek_next() {
    if (pos + 1 < tokens.size()) {
        return &tokens[pos + 1];
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

std::unique_ptr<List> Parser::parse_list() {
    auto and_or = parse_and_or();

    std::vector<ListMid> mid;
    while (peek() && is_list_separator(peek()->type) && peek_next()) {
        mid.push_back(parse_list_mid());
    }

    if (peek() && is_list_separator(peek()->type)) {
        return std::make_unique<List>(
            std::move(and_or), std::move(mid), parse_list_separator()
        );
    }

    return std::make_unique<List>(std::move(and_or), std::move(mid));
}

ListMid Parser::parse_list_mid() {
    return ListMid(parse_list_separator(), parse_and_or());
}

ListSeparator Parser::parse_list_separator() {
    auto &t = advance();
    if (t.type == TokenType::Semicolon) return ListSeparator::Semicolon;
    if (t.type == TokenType::Ampersand) return ListSeparator::Ampersand;

    throw std::runtime_error("unexpected Operator: " + t.text);
}

std::unique_ptr<AndOr> Parser::parse_and_or() {
    auto pipeline = parse_pipeline();

    std::vector<AndOrMid> mid;
    while (peek() && is_and_or_operator(peek()->type)) {
        mid.push_back(parse_and_or_mid());
    }

    return std::make_unique<AndOr>(std::move(pipeline), std::move(mid));
}
AndOrMid Parser::parse_and_or_mid() {
    return AndOrMid(parse_and_or_operator(), parse_pipeline());
}

AndOrOperator Parser::parse_and_or_operator() {
    auto &t = advance();
    if (t.type == TokenType::And) return AndOrOperator::And;
    if (t.type == TokenType::Or) return AndOrOperator::Or;

    throw std::runtime_error("unexpected Operator: " + t.text);
}

std::unique_ptr<Pipeline> Parser::parse_pipeline() {
    auto cmd = parse_cmd();

    std::vector<PipelineMid> mid;
    while (peek() && is_pipeline_operator(peek()->type)) {
        mid.push_back(parse_pipeline_mid());
    }

    return std::make_unique<Pipeline>(std::move(cmd), std::move(mid));
}

PipelineMid Parser::parse_pipeline_mid() {
    return PipelineMid(parse_pipeline_operator(), parse_cmd());
}

PipelineOperator Parser::parse_pipeline_operator() {
    auto &t = advance();
    if (t.type == TokenType::Pipe) return PipelineOperator::Pipe;
    if (t.type == TokenType::ErrPipe) return PipelineOperator::ErrPipe;

    throw std::runtime_error("unexpected Operator: " + t.text);
}

std::vector<std::string> Parser::parse_cmd() {
    std::vector<std::string> cmd;
    do {
        cmd.push_back(expect_word());
    } while (peek() && peek()->type == TokenType::Word);
    return cmd;
}
