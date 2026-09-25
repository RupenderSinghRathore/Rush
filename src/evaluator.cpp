#include "rush/evaluator.hpp"
#include "rush/shell.hpp"

#include "rush/io.hpp"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

int Evaluator::execute_command(
    std::vector<char *> cmd, bool background_process
) {
    auto pid = fork();
    if (pid == 0) {
        execvp(cmd[0], cmd.data());
        auto msg = std::format("unrecognized command: {}\r\n", cmd[0]);
        write_err(std::move(msg));
        _exit(127);
    } else if (pid < 0) {
        write_err("error in fork");
        return STATUS_FAILURE;
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
    return WEXITSTATUS(status);
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

void Evaluator::run(Expr &expr) { last_status = eval_expr(expr); }

int Evaluator::eval_expr(Expr &expr) {
    int status = eval_term(expr.left);

    for (auto op : expr.rest) {
        if (op.first == "&&")
            status = infixop_and(op.second, status);

        else if (op.first == "||")
            status = infixop_or(op.second, status);
    }

    return status;
}

int Evaluator::infixop_and(Term &term, int status) {
    if (status != STATUS_SUCCESS) return status;
    return eval_term(term);
}
int Evaluator::infixop_or(Term &term, int status) {
    if (status == STATUS_SUCCESS) return status;
    return eval_term(term);
}

int Evaluator::eval_term(Term &term) {
    int status = dispatch_builtin(term);
    if (status != STATUS_NOT_A_BUILTIN) return status;

    std::vector<char *> cmd;
    cmd.push_back(term.name.data());
    for (auto arg : term.args) {
        cmd.push_back(arg.data());
    }
    bool background_process = false;
    if (!term.postfix_op.empty()) {
        background_process = true;
    }

    return execute_command(cmd, background_process);
}

int Evaluator::dispatch_builtin(const Term &term) {
    const auto &name = term.name;

    if (name == "cd") {
        return builtin_cd(term.args);
    }

    if (name == "exit") {
        return builtin_exit(term.args);
    }
    return STATUS_NOT_A_BUILTIN;
}

int Evaluator::builtin_exit(const std::vector<std::string> &args) {
    if (args.size() > 1) {
        write_err("exit: too many args");
        return STATUS_FAILURE;
    }
    int status = 0;
    if (args.size() == 1) {
        std::size_t pos{};
        try {
            status = std::stoi(args[0], &pos);
        } catch (std::invalid_argument const &ex) {
            write_err(std::format("exit: {}: invalid argument", args[0]));
            return STATUS_FAILURE;
        } catch (std::out_of_range const &ex) {
            write_err(std::format("exit: {}: out of range", args[0]));
            return STATUS_FAILURE;
        }
    }
    exit = true;
    return status;
}

std::string home() {
    char *home_dir = getenv("HOME");
    if (home_dir == nullptr)
        throw std::runtime_error("error: `HOME` env not set");
    return std::string(home_dir);
}

int Evaluator::builtin_cd(const std::vector<std::string> &args) {
    if (args.size() > 1) {
        write_err("cd: too many args");
        return STATUS_FAILURE;
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
        return STATUS_FAILURE;
    }
    return STATUS_SUCCESS;
}
