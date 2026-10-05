#pragma once

#include "rush/bg_processes.hpp"
#include "rush/parser.hpp"
#include <memory>
#include <optional>
#include <span>

enum class Status {
    Success = 0, // command ran fine
    Failure = 1, // general error (builtins, most failures)
    Usage = 2,   // misuse of shell builtin / bad invocation
    NotExecutable = 126,
    NotFound = 127,
};

class Evaluator {
  private:
    BGProcesses &bg_processes;
    std::string &pwd;
    Status last_status;
    bool &exit;
    int &exit_code;

  public:
    Evaluator(BGProcesses &b, std::string &s, bool &e, int &code)
        : bg_processes(b), pwd(s), exit(e), exit_code(code) {}

    Status execute_command(std::vector<std::string>, bool);
    void run(std::unique_ptr<List> &list);
    Status eval_list(std::unique_ptr<List> &);
    std::optional<Status> eval_list_tail(std::unique_ptr<ListTail> &);
    Status eval_and_or(std::unique_ptr<AndOr> &);
    Status eval_and_or_tail(Status prev, std::unique_ptr<AndOrTail> &);
    Status eval_pipeline(std::unique_ptr<Pipeline> &);
    Status eval_pipeline_tail(std::unique_ptr<PipelineTail> &);
    Status eval_background(std::unique_ptr<Background> &);
    std::optional<Status> eval_background_tail(
        std::unique_ptr<BackgroundTail> &
    );

    std::optional<Status> dispatch_builtin(const std::vector<std::string> &);
    Status builtin_cd(const std::span<const std::string> &);
    Status builtin_exit(const std::span<const std::string> &);
    Status builtin_echo(const std::span<const std::string> &);
};
