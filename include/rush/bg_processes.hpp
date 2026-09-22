#pragma once

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
