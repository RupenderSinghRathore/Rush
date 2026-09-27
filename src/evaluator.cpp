#include "rush/evaluator.hpp"
#include "rush/shell.hpp"

#include "rush/io.hpp"

#include <cstddef>
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
#include <variant>
#include <vector>

Status Evaluator::execute_command(
    std::vector<std::string> command, bool background_process
) {

    if (command.empty()) return Status::Success;

    if (auto status = dispatch_builtin(command)) {
        return status.value();
    }

    std::vector<char *> cmd;
    for (auto &s : command) {
        cmd.push_back(s.data());
    }
    auto pid = fork();
    if (pid == 0) {
        execvp(cmd[0], cmd.data());
        auto msg = std::format("unrecognized command: {}\r\n", cmd[0]);
        write_err(std::move(msg));
        _exit(127);
    } else if (pid < 0) {
        write_err("error in fork");
        return Status::Failure;
    }
    int status = 0;
    if (background_process) {
        write_str(std::format("[{}]\r\n", std::to_string(pid)));
        bg_processes.insert(pid);
    } else {
        waitpid(pid, &status, 0);
        if (status != 0) {
            write_err(
                "returned " + std::to_string(WEXITSTATUS(status)) + "\r\n"
            );
        }
    }
    return static_cast<Status> WEXITSTATUS(status);
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

    // set status
}

Status Evaluator::eval_list(std::unique_ptr<List> &list) {
    auto status = eval_and_or(list->and_or);

    if (list->tail) {
        status = eval_list_tail(list->tail).value_or(status);
    }

    return status;
}

std::optional<Status> Evaluator::eval_list_tail(
    std::unique_ptr<ListTail> &list_tail
) {
    if (!list_tail->inner) {
        return std::nullopt;
    }
    return std::visit(
        [this](auto &ptr) -> std::optional<Status> {
            using T = std::remove_cvref_t<decltype(ptr)>;

            if constexpr (std::is_same_v<T, std::unique_ptr<List>>) {
                return eval_list(ptr);
            } else if constexpr (std::is_same_v<T, std::unique_ptr<ListTail>>) {
                return eval_list_tail(ptr);
            }
        },
        *list_tail->inner
    );
}
Status Evaluator::eval_and_or(std::unique_ptr<AndOr> &and_or) {
    auto status = eval_pipeline(and_or->pipeline);

    if (and_or->and_or_tail) {
        status = eval_and_or_tail(status, and_or->and_or_tail);
    }

    return status;
}
Status Evaluator::eval_and_or_tail(
    Status prev, std::unique_ptr<AndOrTail> &and_or_tail
) {
    auto status = prev;
    if (and_or_tail->op == "&&" && prev == Status::Success) {
        status = eval_pipeline(and_or_tail->pipeline);
    } else if (and_or_tail->op == "||" && prev == Status::Failure) {
        status = eval_pipeline(and_or_tail->pipeline);
    }
    if (and_or_tail->and_or_tail) {
        return eval_and_or_tail(status, and_or_tail->and_or_tail);
    }
    return status;
}

Status Evaluator::eval_pipeline(std::unique_ptr<Pipeline> &pipeline) {
    auto status = eval_background(pipeline->bg);

    if (pipeline->pipeline_tail) {
        // skip for now
        // status = eval_pipeline_tail(pipeline->pipeline_tail);
    }
    return status;
}

Status Evaluator::eval_background(std::unique_ptr<Background> &background) {
    auto status =
        execute_command(background->cmd, (background->bg_tail != nullptr));
    if (background->bg_tail) {
        status = eval_background_tail(background->bg_tail).value_or(status);
    }
    return status;
}
std::optional<Status> Evaluator::eval_background_tail(
    std::unique_ptr<BackgroundTail> &background_tail
) {
    if (background_tail->list) return eval_list(background_tail->list);
    return std::nullopt;
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
