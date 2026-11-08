#include "rush/tokenizer.hpp"
#include <cassert>
#include <vector>

bool token_equal(const std::vector<Token> &t1, const std::vector<Token> &t2) {
    if (t1.size() != t2.size()) return false;

    for (size_t i = 0; i < t1.size(); i++) {
        if (t1[i].type != t2[i].type || t1[i].text != t2[i].text) return false;
    }

    return true;
}

void test_tokenize() {
    Tokenizer t{};
    auto input = "ls -la";
    auto expect_tokens = std::vector<Token>{
        Token{TokenType::Word, "ls"}, Token{TokenType::Word, "-la"}
    };

    t.tokenize(input);

    if (!token_equal(expect_tokens, t.tokens)) {
    }

    assert(1);
}

int main(void) {
    test_tokenize();
    return 0;
}
