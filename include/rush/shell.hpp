#pragma once

#include "rush/editor.hpp"
#include "rush/parser.hpp"
#include "rush/terminal.hpp"
#include "rush/tokenizer.hpp"

#include <vector>

class Shell {
  private:
    Terminal term;
    Tokenizer tokenizer;
    Parser parser;
    Editor editor;

  public:
    Shell() = default;

    void repl();
    void eval_key(Key key);
    void execute_command(std::vector<char *> cmd);
    void eval_expr(Expr expr);
    void eval_term(Term term);
};
