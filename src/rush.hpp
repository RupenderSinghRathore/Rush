#pragma once

#include <cstdio>
#include <format>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define PROMPT "> "
#define QUOTED_PROMPT " "

class Terminal {
  private:
    termios original;
    termios raw;

  public:
    Terminal();
    void enter_raw_mode();
    void exit_raw_mode();
};

enum class TokenType {
    Identifier = 0,
    Constant,
    Infix_Operator,
    Postfix_Operator,
    Unknown
};
struct Token {
    TokenType type;
    std::string text;
};
class Tokenizer {
  public:
    std::vector<Token> tokens;
    Token token;
    char quote = '\0';
    bool started = false;

    Tokenizer() = default;

    void consume_char(char c);
    bool finish_tokenizer();
};

/* GRAMMER

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
struct Start {
    Expr expr;
};

class Parser {
  public:
    std::vector<Token> tokens;
    size_t pos;

    Parser() = default;
    void reset(std::vector<Token> t) {
        tokens = std::move(t);
        pos = 0;
    }
    Expr parse_start() {
        auto expr = parse_expr();
        if (peek()) {
            throw std::runtime_error(
                "unexpected trailing tokens: " + peek()->text
            );
        }
        return expr;
    }
    const Token *peek() {
        if (pos < tokens.size()) {
            return &tokens[pos];
        }
        return nullptr;
    }
    Token advance() {
        if (pos >= tokens.size()) {
            throw std::runtime_error("unexpected end of input");
        }
        return tokens[pos++];
    }
    std::string expect_identifier() {
        Token t = advance();
        if (t.type != TokenType::Identifier) {
            throw std::runtime_error("expected identifier, got: " + t.text);
        }
        return t.text;
    }
    std::string expect_constant() {
        Token t = advance();
        if (t.type != TokenType::Constant) {
            throw std::runtime_error("expected Constant, got: " + t.text);
        }
        return t.text;
    }
    std::string expect_infix_operator() {
        Token t = advance();
        if (t.type != TokenType::Infix_Operator) {
            throw std::runtime_error("expected Operator, got: " + t.text);
        }
        return t.text;
    }
    std::string expect_postfix_operator() {
        Token t = advance();
        if (t.type != TokenType::Postfix_Operator) {
            throw std::runtime_error("expected Operator, got: " + t.text);
        }
        return t.text;
    }
    Expr parse_expr() {
        auto left = parse_term();
        std::vector<std::pair<std::string, Term>> rest;
        while (peek() && peek()->type == TokenType::Infix_Operator) {
            auto op = expect_infix_operator();
            auto term = parse_term();
            rest.push_back({std::move(op), std::move(term)});
        }
        return Expr(std::move(left), std::move(rest));
    }
    Term parse_term() {
        auto term = Term{};
        term.name = expect_identifier();
        term.args = parse_args();
        if (peek() && peek()->type == TokenType::Postfix_Operator) {
            term.postfix_op = expect_postfix_operator();
        }
        return term;
    }
    std::vector<std::string> parse_args() {
        std::vector<std::string> args;
        while (peek() && peek()->type == TokenType::Constant) {
            args.push_back(expect_constant());
        }
        return args;
    }
};

class Shell {
  private:
    Terminal term;
    Tokenizer tokenizer;
    Parser parser;

  public:
    Shell() = default;

    void repl();
    void execute_command();
    void write_char(char c);
    void write_str(const std::string &s);

    template <typename... Args>
    void write_err(std::format_string<Args...> fmt, Args &&...args);
};
