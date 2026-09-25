#pragma once

#include "rush/bg_processes.hpp"
#include "rush/parser.hpp"

constexpr int STATUS_NOT_A_BUILTIN = -1; // command ran fine
constexpr int STATUS_SUCCESS = 0;        // command ran fine
constexpr int STATUS_FAILURE = 1; // general error (builtins, most failures)
constexpr int STATUS_USAGE = 2;   // misuse of shell builtin / bad invocation
constexpr int STATUS_NOT_EXECUTABLE =
    126; // found, but couldn't exec (permissions, not a binary)
constexpr int STATUS_NOT_FOUND = 127; // command not found / couldn't resolve

class Evaluator {
  private:
    BGProcesses &bg_processes;
    std::string &pwd;
    int last_status;
    bool &exit;

  public:
    Evaluator(BGProcesses &b, std::string &s, bool &e)
        : bg_processes(b), pwd(s), exit(e) {}

    int execute_command(std::vector<char *> cmd, bool background_process);
    void run(Expr &expr);
    int eval_expr(Expr &expr);
    int eval_term(Term &term);

    int dispatch_builtin(const Term &term);
    int builtin_cd(const std::vector<std::string> &args);
    int builtin_exit(const std::vector<std::string> &args);
    int builtin_echo(const std::vector<std::string> &args);
    // void builtin_cd(std::vector<std::string> args);

    int infixop_and(Term &term, int status);
    int infixop_or(Term &term, int status);
};
