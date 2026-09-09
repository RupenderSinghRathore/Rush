#include <iostream>
#include <print>
#include <string>

int main(int argc, char *argv[]) {
    while (1) {
        std::printf("> ");
        std::string input;
        std::getline(std::cin, input);

        // TODO: compiler phases

        std::println("got: {}", input);
    }
    return 0;
}
