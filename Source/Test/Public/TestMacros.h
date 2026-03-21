#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Test result
struct FTestResult {
    std::string Name;
    bool Passed = true;
    std::string ErrorMessage;
    double DurationMs = 0.0;
};

// Test suite
class FTestSuite {
public:
    using TestFunc = std::function<void()>;

    void AddTest(const std::string& name, TestFunc func) { m_Tests.push_back({ name, func }); }

    std::vector<FTestResult> Run() {
        std::vector<FTestResult> results;
        for (auto& [name, func] : m_Tests) {
            FTestResult result;
            result.Name = name;

            try {
                auto start = std::chrono::high_resolution_clock::now();
                func();
                auto end = std::chrono::high_resolution_clock::now();
                result.DurationMs = std::chrono::duration<double, std::milli>(end - start).count();
            } catch (const std::exception& e) {
                result.Passed = false;
                result.ErrorMessage = e.what();
            }

            results.push_back(result);
        }
        return results;
    }

private:
    struct FTest {
        std::string Name;
        TestFunc Func;
    };
    std::vector<FTest> m_Tests;
};

// Global test registration
inline std::vector<std::pair<std::string, std::function<void(FTestSuite&)>>>&
    GetTestRegistrations() {
    static std::vector<std::pair<std::string, std::function<void(FTestSuite&)>>> registrations;
    return registrations;
}

// Test macros
#define TEST_SUITE(name)                                                                           \
    static void Run##name##Tests(FTestSuite& suite);                                               \
    namespace {                                                                                    \
    struct Register##name {                                                                        \
        Register##name() { GetTestRegistrations().push_back({ #name, Run##name##Tests }); }        \
    } register##name;                                                                              \
    }                                                                                              \
    void Run##name##Tests(FTestSuite& suite)

#define TEST(name) suite.AddTest(#name, [&]()
#define TEST_END );