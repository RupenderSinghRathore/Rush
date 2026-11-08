#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ==========================================
// TEST REGISTRY & FRAMEWORK
// ==========================================

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i] << (i + 1 < vec.size() ? ", " : "");
    }
    os << "]";
    return os;
}

struct TestRegistry {
    struct TestCase {
        std::string name;
        std::function<void()> func;
    };

    std::vector<TestCase> tests;
    int passed = 0;
    int failed = 0;

    static TestRegistry &instance() {
        static TestRegistry reg;
        return reg;
    }

    // Explicitly run all registered tests
    void run_all() {
        for (const auto &test : tests) {
            std::cout << "[RUN ] " << test.name << "\n";
            int prev_failed = failed;
            test.func();
            if (failed == prev_failed) {
                std::cout << "  [OK  ] " << test.name << "\n";
                passed++;
            }
        }
    }
};

// container check for deep_equal
template <typename, typename = std::void_t<>>
struct is_container : std::false_type {};

template <typename T>
struct is_container<
    T,
    std::void_t<
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end()),
        typename T::value_type>> : std::true_type {};

template <typename T> bool deep_equal(const T &a, const T &b);

template <typename T> bool deep_equal_container(const T &a, const T &b) {
    if (a.size() != b.size()) return false;
    auto it1 = a.begin();
    auto it2 = b.begin();
    while (it1 != a.end()) {
        if (!deep_equal(*it1, *it2)) return false;
        ++it1;
        ++it2;
    }
    return true;
}

template <typename T> bool deep_equal(const T &a, const T &b) {
    if constexpr (is_container<T>::value && !std::is_same_v<T, std::string>) {
        return deep_equal_container(a, b);
    } else {
        return a == b;
    }
}

// Macros
#define ASSERT_EQ(actual, expected)                                            \
    do {                                                                       \
        auto act = (actual);                                                   \
        auto exp = (expected);                                                 \
        if (!deep_equal(act, exp)) {                                           \
            std::cerr << "  [FAIL] " << __FILE__ << ":" << __LINE__ << "\n"    \
                      << " -> Assertion failed: " << #actual                   \
                      << " == " << #expected << "\n\n";                        \
            std::cerr << "\tactual: " << act << "\n";                          \
            std::cerr << "\texpected: " << exp << "\n";                        \
            TestRegistry::instance().failed++;                                 \
            return;                                                            \
        }                                                                      \
    } while (false)

// only registers the test instead of running it instantly
#define TEST(test_name)                                                        \
    void test_name();                                                          \
    struct Register_##test_name {                                              \
        Register_##test_name() {                                               \
            TestRegistry::instance().tests.push_back({#test_name, test_name}); \
        }                                                                      \
    } initiator_##test_name;                                                   \
    void test_name()
