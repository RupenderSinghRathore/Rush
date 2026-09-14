#include "rush/shell.hpp"

#include "rush/io.hpp"

#include <cstdlib>
#include <exception>
#include <format>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void Shell::repl() {
    term.enter_raw_mode();
    write_str("> ");
    while (1) {
        auto key = term.read_key();
        eval_key(key);
    }
    term.exit_raw_mode();
}

void Shell::eval_key(Key key) {

    switch (key.type) {
    case KeyType::Backspace:
        editor.delete_char();
        term.draw(editor.text(), editor.cursor());
        break;

    case KeyType::Ctrl:
        switch (key.ch) {
        case 'H':
            editor.delete_char();
            term.draw(editor.text(), editor.cursor());
            break;
        case 'Q':
            term.exit_raw_mode();
            exit(0);
        }
        break;

    case KeyType::Enter: {
        term.new_line();
        if (editor.is_empty()) {
            term.draw();
            break;
        }
        editor.insert('\r');
        editor.insert('\n');
        tokenizer.tokenize(editor.text());
        if (tokenizer.is_quoted()) {
            write_char(' ');
            tokenizer.clear();
            break;
        }
        editor.clear();

        if (!tokenizer.is_empty()) {
            try {
                parser.reset(tokenizer.owned_tokens());
                auto expr = parser.parse_start();
                term.exit_raw_mode();
                eval_expr(expr);
            } catch (std::exception &e) {
                term.exit_raw_mode();
                write_err(e.what());
                exit(1);
            }
            term.enter_raw_mode();
        }
        tokenizer.clear();
        write_str("> ");
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

void Shell::execute_command(std::vector<char *> cmd) {
    auto pid = fork();
    if (pid == 0) {
        execvp(cmd[0], cmd.data());
        auto msg = std::format("unrecognized command: {}\n", cmd[0]);
        write_err(msg);
        _exit(127);
    } else if (pid < 0) {
        perror("error in fork");
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
        write_str("returned " + std::to_string(WEXITSTATUS(status)) + "\n");
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
    // TODO: process postfix operators

    execute_command(cmd);
}
