#pragma once

#include <cstddef>
#include <string>

class Editor {
  private:
    std::string line;
    size_t index;

  public:
    Editor() = default;
    void insert(char ch);
    void insert(std::string s);
    std::string text();
    std::string owned_text();
    size_t cursor();
    void delete_char();
    void clear();
    bool is_empty();
    char curr_char();
};
