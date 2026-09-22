#pragma once

#include "rush/bg_processes.hpp"
#include "rush/parser.hpp"

class Evaluator {
  private:
    BGProcesses &bg_processes;

  public:
    Evaluator(BGProcesses &b) : bg_processes(b) {}

    void execute_command(std::vector<char *> cmd, bool background_process);
    void eval_expr(Expr &expr);
    void eval_term(Term &term);

    bool dispatch_builtin(const Term &term);
    void builtin_cd(const std::vector<std::string> &args);
    void builtin_exit(const std::vector<std::string> &args);
    void builtin_echo(const std::vector<std::string> &args);
    // void builtin_cd(std::vector<std::string> args);
};
