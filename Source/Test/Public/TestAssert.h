#pragma once

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "CoreAssert.h"

namespace {
struct AutoEnableCoreAssertThrow {
    AutoEnableCoreAssertThrow() { CZ_ENABLE_ASSERT_THROW(); }
};
static AutoEnableCoreAssertThrow s_AutoEnableCoreAssertThrow;
} // namespace

inline void Assert(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        std::stringstream ss;
        ss << "Assertion failed: " << expr << " at " << file << ":" << line;
        throw std::runtime_error(ss.str());
    }
}

template <typename T, typename U>
void AssertEqual(const T& expected, const U& actual, const char* expr_expected,
                 const char* expr_actual, const char* file, int line) {
    if (expected != actual) {
        std::stringstream ss;
        ss << "Expected: " << expr_expected << " = " << expected << "\n";
        ss << "Actual: " << expr_actual << " = " << actual << "\n";
        ss << "at " << file << ":" << line;
        throw std::runtime_error(ss.str());
    }
}

#define TEST_ASSERT(cond) Assert(cond, #cond, __FILE__, __LINE__)
#define TEST_ASSERT_EQ(expected, actual)                                                           \
    AssertEqual(expected, actual, #expected, #actual, __FILE__, __LINE__)
#define TEST_ASSERT_NE(expected, actual) TEST_ASSERT(!((expected) == (actual)))
#define TEST_ASSERT_GT(a, b) TEST_ASSERT((a) > (b))
#define TEST_ASSERT_LT(a, b) TEST_ASSERT((a) < (b))
#define TEST_ASSERT_GE(a, b) TEST_ASSERT((a) >= (b))
#define TEST_ASSERT_LE(a, b) TEST_ASSERT((a) <= (b))
#define TEST_ASSERT_NEAR(a, b, epsilon) TEST_ASSERT(std::abs((a) - (b)) <= (epsilon))

#define TEST_EXPECT_FAIL(expr)                                                                     \
    do {                                                                                           \
        bool caught = false;                                                                       \
        try {                                                                                      \
            expr;                                                                                  \
        } catch (const std::exception&) {                                                          \
            caught = true;                                                                         \
        }                                                                                          \
        if (!caught) {                                                                             \
            throw std::runtime_error("Expected assertion did not trigger");                        \
        }                                                                                          \
    } while (0)

#define TEST_EXPECT_FAIL_WITH_MSG(expr, expectedMsg)                                               \
    do {                                                                                           \
        bool caught = false;                                                                       \
        std::string actualMsg;                                                                     \
        try {                                                                                      \
            expr;                                                                                  \
        } catch (const std::exception& e) {                                                        \
            caught = true;                                                                         \
            actualMsg = e.what();                                                                  \
        }                                                                                          \
        if (!caught) {                                                                             \
            throw std::runtime_error("Expected assertion did not trigger");                        \
        }                                                                                          \
        if (actualMsg.find(expectedMsg) == std::string::npos) {                                    \
            std::stringstream ss;                                                                  \
            ss << "Expected message containing: " << expectedMsg << "\n";                          \
            ss << "Actual message: " << actualMsg;                                                 \
            throw std::runtime_error(ss.str());                                                    \
        }                                                                                          \
    } while (0)