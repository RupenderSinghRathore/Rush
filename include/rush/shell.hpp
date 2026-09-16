#pragma once

#include "rush/editor.hpp"
#include "rush/parser.hpp"
#include "rush/terminal.hpp"
#include "rush/tokenizer.hpp"

#include <vector>

struct BGProcess {
    int pid;
    BGProcess *next = nullptr;
    BGProcess *prev = nullptr;
};

class BGProcesses {
  public:
    BGProcess *root = nullptr;
    BGProcesses() = default;

    void insert(int pid) {
        auto new_node = new BGProcess(pid);
        if (!root) {
            root = new_node;
            return;
        }
        new_node->next = root;
        root->prev = new_node;
    }
    void remove(BGProcess *node) {
        if (!node) return;

        if (node->prev)
            node->prev->next = node->next;
        else
            root = node->next;

        if (node->next) node->next->prev = node->prev;

        delete node;
    }
};

class Shell {
  private:
    Terminal term;
    Tokenizer tokenizer;
    Parser parser;
    Editor editor;
    BGProcesses bg_processes;

  public:
    Shell() = default;

    void repl();
    void eval_key(Key key);
    void handle_backspace();
    void handle_enter();
    void reap_bg_processes();

    void execute_command(std::vector<char *> cmd, bool background_process);
    void eval_expr(Expr expr);
    void eval_term(Term term);
};
