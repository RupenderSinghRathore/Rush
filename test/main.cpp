#include "test_framework.hpp"

int main() {
    std::cout << "=== STARTING UNIT TESTS ===\n\n";

    TestRegistry::instance().run_all();

    auto reg = TestRegistry::instance();
    std::cout << "\n=== TEST SUMMARY ===\n";
    std::cout << "Passed: " << reg.passed << "\n";
    std::cout << "Failed: " << reg.failed << "\n";

    return (reg.failed == 0) ? 0 : 1;
}
