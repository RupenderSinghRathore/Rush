#include "rush/editor.hpp"

#include <utility>

void Editor::insert(char ch) { line.insert(line.begin() + index++, ch); }

std::string Editor::text() { return line; }

std::string Editor::owned_text() { return std::move(line); }

size_t Editor::cursor() { return index; }

void Editor::delete_char() {
    if (index == 0) return;
    if (line[index - 1] == '\n') {
        return;
    }
    line.erase(line.begin() + --index);
}

void Editor::clear() {
    line.clear();
    index = 0;
}

bool Editor::is_empty() { return line.empty(); }
