#pragma once

#include "rush/bg_processes.hpp"
#include "rush/editor.hpp"
#include "rush/evaluator.hpp"
#include "rush/parser.hpp"
#include "rush/terminal.hpp"
#include "rush/tokenizer.hpp"
#include <filesystem>

class Shell {
  private:
    std::string pwd;
    bool needs_redraw = true;
    Terminal term;
    Tokenizer tokenizer;
    Parser parser;
    Editor editor;
    BGProcesses bg_processes;
    Evaluator eval;

  public:
    Shell() : term(pwd), eval(bg_processes, pwd) {
        pwd = std::filesystem::current_path();
    }

    void repl();
    void eval_key(Key key);
    void handle_backspace();
    void handle_enter();
    void reap_bg_processes();
};
