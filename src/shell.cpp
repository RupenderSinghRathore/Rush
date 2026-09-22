#include "rush/shell.hpp"

#include "rush/io.hpp"
#include "rush/terminal.hpp"

#include <cstdlib>
#include <exception>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

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
            eval.eval_expr(expr);
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
