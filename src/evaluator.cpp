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

void Evaluator::execute_command(
    std::vector<char *> cmd, bool background_process
) {
    auto pid = fork();
    if (pid == 0) {
        execvp(cmd[0], cmd.data());
        auto msg = std::format("unrecognized command: {}\r\n", cmd[0]);
        write_err(std::move(msg));
        _exit(127);
    } else if (pid < 0) {
        perror("error in fork");
        exit(1);
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

void Evaluator::eval_expr(Expr &expr) {
    eval_term(expr.left);

    // TODO: Process rest of expr
}

void Evaluator::eval_term(Term &term) {
    if (dispatch_builtin(term)) {
        return;
    }
    std::vector<char *> cmd;
    cmd.push_back(term.name.data());
    for (auto arg : term.args) {
        cmd.push_back(arg.data());
    }
    bool background_process = false;
    if (!term.postfix_op.empty()) {
        // write_str("using " + term.postfix_op + " operator");
        background_process = true;
    }

    execute_command(cmd, background_process);
}

bool Evaluator::dispatch_builtin(const Term &term) {
    const auto &name = term.name;

    if (name == "cd") {
        builtin_cd(term.args);
        return true;
    }

    if (name == "exit") {
        builtin_exit(term.args);
        return true;
    }
    return false;
}

void Evaluator::builtin_exit(const std::vector<std::string> &args) {
    if (args.size() > 1) {
        write_err("exit: too many args");
        return;
    }
    int status = 0;
    if (args.size() == 1) {
        std::size_t pos{};
        try {
            status = std::stoi(args[0], &pos);
        } catch (std::invalid_argument const &ex) {
            write_err(std::format("exit: {}: invalid argument", args[0]));
            return;
        } catch (std::out_of_range const &ex) {
            write_err(std::format("exit: {}: out of range", args[0]));
            return;
        }
    }
    exit(status);
}

std::string home() {
    char *home_dir = getenv("HOME");
    if (home_dir == nullptr)
        throw std::runtime_error("error: `HOME` env not set");
    return std::string(home_dir);
}

void Evaluator::builtin_cd(const std::vector<std::string> &args) {
    if (args.size() > 1) {
        write_err("cd: too many args");
        return;
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
    }
}
