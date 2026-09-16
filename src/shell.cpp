#include "rush/shell.hpp"

#include "rush/io.hpp"
#include "rush/terminal.hpp"

#include <cstdlib>
#include <exception>
#include <format>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void Shell::repl() {
    term.enter_raw_mode();
    write_str(PROMPT);
    while (1) {
        auto key = term.read_key();
        eval_key(key);
    }
    term.exit_raw_mode();
}

void Shell::handle_backspace() {
    editor.delete_char();
    term.draw(editor.text(), editor.cursor());
}

void Shell::handle_enter() {
    term.new_line();
    if (editor.is_empty()) {
        goto redraw;
    }
    editor.insert("\r\n");
    try {
        tokenizer.tokenize(editor.text());
    } catch (std::exception &e) {
        write_err(e.what());
        term.new_line();
        goto redraw;
    }
    if (tokenizer.is_quoted()) {
        term.increment_row();
        tokenizer.clear();
        return;
    }

    if (!tokenizer.is_empty()) {
        try {
            parser.reset(tokenizer.owned_tokens());
            auto expr = parser.parse_start();
            term.exit_raw_mode();
            eval_expr(expr);
        } catch (std::exception &e) {
            write_err(e.what());
        }
        term.enter_raw_mode();
    }

redraw:
    editor.clear();
    tokenizer.clear();
    term.reset_row();
    reap_bg_processes();
    term.draw();
}

void Shell::eval_key(Key key) {

    switch (key.type) {
    case KeyType::Backspace:
        handle_backspace();
        break;

    case KeyType::Ctrl:
        switch (key.ch) {
        case 'H':
            handle_backspace();
            break;
        case 'C':
            editor.clear();
            term.draw();
            term.reset_row();
            break;
        case 'M':
            handle_enter();
            break;
        case 'Q':
            term.exit_raw_mode();
            exit(0);
        }
        break;

    case KeyType::Enter: {
        handle_enter();
        break;
    }

    case KeyType::Char:
        editor.insert(key.ch);
        write_char(key.ch);
        break;

    case KeyType::ArrowUp: /* history later */
        break;
    case KeyType::ArrowDown: /* history later */
        break;
    case KeyType::ArrowLeft: /* cursor move later */
        break;
    case KeyType::ArrowRight: /* cursor move later */
        break;
    case KeyType::Delete: /* forward delete */
        break;
    default:
        break;
    }
}

void Shell::execute_command(std::vector<char *> cmd, bool background_process) {
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

void Shell::eval_expr(Expr expr) {
    eval_term(expr.left);

    // TODO: Process rest of expr
}

void Shell::eval_term(Term term) {
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
