#include "rush/evaluator.hpp"
#include "rush/parser.hpp"
#include "rush/shell.hpp"

#include "rush/io.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <wait.h>

Status Evaluator::execute_command(std::vector<std::string> command) {
    if (command.empty()) return Status::Success;

    if (auto status = dispatch_builtin(command)) {
        return *status;
    }

    auto pid = fork();

    if (pid == 0) {
        auto cmd = prep_cmd(command);
        execvp(cmd[0], cmd.data());
        _exit(127);
    }

    if (pid < 0) {
        write_err("error in fork");
        return Status::Failure;
    }

    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        write_err("error in waitpid");
        return Status::Failure;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return Status::Success;
    }

    return Status::Failure;
}

Status Evaluator::execute_pipe(std::unique_ptr<Pipeline> &pipeline) {
    auto cmd_count = pipeline->mid.size() + 1;
    std::vector<pid_t> pids;
    pids.reserve(cmd_count);

    int prev_read = -1;

    for (size_t i = 0; i < cmd_count; i++) {
        auto &cmd = (i == 0) ? pipeline->cmd : pipeline->mid[i - 1].cmd;
        int fd[2] = {-1, -1};
        const bool has_next = i + 1 < cmd_count;
        if (has_next) pipe(fd);

        auto pid = fork();
        if (pid == 0) {
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
            }
            if (has_next) {
                dup2(fd[1], STDOUT_FILENO);
                if (pipeline->mid[i].op == PipelineOperator::ErrPipe)
                    dup2(fd[1], STDERR_FILENO);
            }

            close(prev_read);
            close(fd[0]);
            close(fd[1]);

            if (auto status = dispatch_builtin(cmd)) {
                _exit(static_cast<int>(*status));
            }
            auto preped = prep_cmd(cmd);
            execvp(preped[0], preped.data());
            _exit(127);
        }
        pids.push_back(pid);

        close(fd[1]);
        close(prev_read);

        if (has_next) prev_read = fd[0];
    }

    int status = 0;
    for (auto pid : pids) {
        while (waitpid(pid, &status, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("waitpid");
            return Status::Failure;
        }
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return Status::Success;
    }

    return Status::Failure;
}

std::vector<char *> Evaluator::prep_cmd(std::vector<std::string> &cmd) {
    std::vector<char *> argv;
    argv.reserve(cmd.size() + 1);

    for (auto &arg : cmd)
        argv.push_back(arg.data());

    argv.push_back(nullptr);
    return argv;
}

void Shell::reap_bg_processes() {
    auto curr = bg_processes.root;
    while (curr) {
        int status = 0;
        auto ret = waitpid(curr->pid, &status, WNOHANG);
        if (ret == curr->pid && (WIFEXITED(status) || WIFSIGNALED(status))) {
            if (WIFEXITED(status)) {
                write_str(
                    std::format(
                        "[{}], exited with status: {}\r\n",
                        std::to_string(curr->pid),
                        std::to_string(WEXITSTATUS(status))
                    )
                );
            } else {
                write_str(
                    std::format(
                        "[{}], killed by signal: {}\r\n",
                        std::to_string(curr->pid),
                        std::to_string(WTERMSIG(status))
                    )
                );
            }
            auto next = curr->next;
            bg_processes.remove(curr);
            curr = next;
        } else {
            curr = curr->next;
        }
    }
}

void Evaluator::run(std::unique_ptr<List> &list) {
    last_status = eval_list(list);
}

Status Evaluator::eval_list(std::unique_ptr<List> &list) {

    auto bg =
        (list->mid.empty() ? list->sep.value_or(ListSeparator::Semicolon)
                           : list->mid[0].sep) == ListSeparator::Ampersand;

    auto status =
        bg ? start_bg_process(list->and_or) : eval_and_or(list->and_or);

    for (size_t i = 0; i < list->mid.size(); i++) {
        bg = ((i + 1 < list->mid.size())
                  ? list->mid[i + 1].sep
                  : list->sep.value_or(ListSeparator::Semicolon)) ==
             ListSeparator::Ampersand;

        status = bg ? start_bg_process(list->mid[i].and_or)
                    : eval_and_or(list->mid[i].and_or);
    }

    return status;
}

Status Evaluator::start_bg_process(std::unique_ptr<AndOr> &and_or) {
    auto pid = fork();
    if (pid == 0) {
        auto status = eval_and_or(and_or);
        _exit(static_cast<int>(status));
    }

    if (pid < 0) {
        write_err("error in fork");
        return Status::Failure;
    }

    write_str(std::format("[{}]\r\n", std::to_string(pid)));
    bg_processes.insert(pid);

    return Status::Success;
}

Status Evaluator::eval_and_or(std::unique_ptr<AndOr> &and_or) {

    auto status = eval_pipeline(and_or->pipeline);

    auto &mid = and_or->mid;
    for (size_t i = 0; i < mid.size(); i++) {
        auto op = mid[i].op;
        if (op == AndOrOperator::And && status == Status::Success) {
            status = eval_pipeline(mid[i].pipeline);
            continue;
        }

        if (op == AndOrOperator::Or && status == Status::Failure) {
            status = eval_pipeline(mid[i].pipeline);
            continue;
        }
    }

    return status;
}

Status Evaluator::eval_pipeline(std::unique_ptr<Pipeline> &pipeline) {
    return pipeline->mid.empty() ? execute_command(pipeline->cmd)
                                 : execute_pipe(pipeline);
}

std::optional<Status> Evaluator::dispatch_builtin(
    const std::vector<std::string> &cmd
) {
    auto name = cmd[0];
    std::span<const std::string> args(cmd.data() + 1, cmd.size() - 1);

    if (name == "cd") {
        return builtin_cd(std::move(args));
    }

    if (name == "exit") {
        return builtin_exit(std::move(args));
    }
    return std::nullopt;
}

Status Evaluator::builtin_exit(const std::span<const std::string> &args) {
    if (args.size() > 1) {
        write_err("exit: too many args");
        return Status::Failure;
    }
    int code = 0;
    if (args.size() == 1) {
        std::size_t pos{};
        try {
            code = std::stoi(args[0], &pos);
        } catch (std::invalid_argument const &ex) {
            write_err(std::format("exit: {}: invalid argument", args[0]));
            return Status::Failure;
        } catch (std::out_of_range const &ex) {
            write_err(std::format("exit: {}: out of range", args[0]));
            return Status::Failure;
        }
    }
    exit = true;
    exit_code = code;

    return Status::Success;
}

std::string home() {
    char *home_dir = getenv("HOME");
    if (home_dir == nullptr)
        throw std::runtime_error("error: `HOME` env not set");
    return std::string(home_dir);
}

Status Evaluator::builtin_cd(const std::span<const std::string> &args) {
    if (args.size() > 1) {
        write_err("cd: too many args");
        return Status::Failure;
    }
    std::string dir;
    if (args.size() == 1) {
        dir = args[0];
    } else {
        dir = home();
    }
    try {
        std::filesystem::current_path(dir);
        pwd = std::filesystem::current_path();
    } catch (const std::filesystem::filesystem_error &e) {
        write_err(std::format("cd: {}", e.what()));
        return Status::Failure;
    }
    return Status::Success;
}
