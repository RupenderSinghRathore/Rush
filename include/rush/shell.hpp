#pragma once

#include "rush/bg_processes.hpp"
#include "rush/editor.hpp"
#include "rush/executor.hpp"
#include "rush/parser.hpp"
#include "rush/terminal.hpp"
#include "rush/tokenizer.hpp"

class Shell {
  private:
    Terminal term;
    Tokenizer tokenizer;
    Parser parser;
    Editor editor;
    BGProcesses bg_processes;
    Evaluator eval;

  public:
    Shell() : eval(bg_processes) {}

    void repl();
    void eval_key(Key key);
    void handle_backspace();
    void handle_enter();
    void reap_bg_processes();
};
