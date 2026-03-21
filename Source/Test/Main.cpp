// Source/Test/Main.cpp
#include "TestAssert.h"
#include "TestMacros.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

extern std::vector<std::pair<std::string, std::function<void(FTestSuite&)>>>&
    GetTestRegistrations();

// ===== Command line argument parsing =====
struct TestOptions {
    std::string Filter = "";
    bool Verbose = false;
    bool ListTests = false;
};

TestOptions ParseArgs(int argc, char* argv[]) {
    TestOptions options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--gtest_filter" && i + 1 < argc) {
            options.Filter = argv[++i];
        } else if (arg == "--filter" && i + 1 < argc) {
            options.Filter = argv[++i];
        } else if (arg.substr(0, 9) == "--filter=") {
            options.Filter = arg.substr(9);
        } else if (arg == "-v" || arg == "--verbose") {
            options.Verbose = true;
        } else if (arg == "--list-tests") {
            options.ListTests = true;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: Test [options]\n"
                      << "Options:\n"
                      << "  --filter=<pattern>   Run only tests matching pattern\n"
                      << "  -v, --verbose        Verbose output\n"
                      << "  --list-tests         List all tests without running\n"
                      << "  -h, --help           Show this help\n";
            exit(0);
        }
    }
    return options;
}

// ===== Pattern matching =====
bool MatchesFilter(const std::string& name, const std::string& filter) {
    if (filter.empty()) return true;

    // Support wildcard (*) at beginning or end
    if (filter.front() == '*' && filter.back() == '*') {
        std::string pattern = filter.substr(1, filter.length() - 2);
        return name.find(pattern) != std::string::npos;
    } else if (filter.front() == '*') {
        std::string suffix = filter.substr(1);
        return name.length() >= suffix.length() &&
               name.compare(name.length() - suffix.length(), suffix.length(), suffix) == 0;
    } else if (filter.back() == '*') {
        std::string prefix = filter.substr(0, filter.length() - 1);
        return name.compare(0, prefix.length(), prefix) == 0;
    } else {
        return name == filter;
    }
}

// ===== Helper functions =====
void PrintSeparator(char c = '=', int width = 60) {
    std::cout << std::string(width, c) << std::endl;
}

void PrintSuccess(const std::string& text) { std::cout << "\033[32m" << text << "\033[0m"; }

void PrintError(const std::string& text) { std::cout << "\033[31m" << text << "\033[0m"; }

void PrintWarning(const std::string& text) { std::cout << "\033[33m" << text << "\033[0m"; }

void PrintInfo(const std::string& text) { std::cout << "\033[36m" << text << "\033[0m"; }

void PrintBold(const std::string& text) { std::cout << "\033[1m" << text << "\033[0m"; }

int main(int argc, char* argv[]) {
    auto options = ParseArgs(argc, argv);

    auto& registrations = GetTestRegistrations();

    // List tests mode
    if (options.ListTests) {
        std::cout << "Registered test suites: " << registrations.size() << std::endl;
        for (auto& [suiteName, _] : registrations) {
            std::cout << "  " << suiteName << std::endl;
        }
        return 0;
    }

    // Header
    PrintSeparator('=', 60);
    PrintBold("        Chozo Engine Test Suite");
    std::cout << std::endl;
    PrintSeparator('=', 60);
    std::cout << std::endl;

    PrintInfo("📋 Test Suites Registered: " + std::to_string(registrations.size()));
    std::cout << std::endl;

    // Show filter if applied
    if (!options.Filter.empty()) {
        PrintInfo("🔍 Filter: " + options.Filter);
        std::cout << std::endl;
    }

    if (options.Verbose) {
        for (auto& [name, _] : registrations) {
            std::cout << "  - " << name << std::endl;
        }
        std::cout << std::endl;
    }

    if (registrations.empty()) {
        PrintWarning("⚠️  No tests registered!");
        std::cout << std::endl;
        return 0;
    }

    int totalPassed = 0;
    int totalFailed = 0;
    double totalTime = 0.0;
    std::vector<std::string> failedTests;
    int suitesRun = 0;

    // Run each test suite
    for (auto& [suiteName, registration] : registrations) {
        // Check filter
        bool shouldRun = MatchesFilter(suiteName, options.Filter);

        // if (options.Verbose) {
        //     std::cout << "Filter check: '" << suiteName << "' vs '" << options.Filter << "' -> "
        //               << (shouldRun ? "RUN" : "SKIP") << std::endl;
        // }

        if (!shouldRun) {
            continue;
        }

        suitesRun++;

        FTestSuite suite;
        registration(suite);

        std::cout << std::endl;
        PrintSeparator('-', 60);
        PrintBold("▶ " + suiteName);
        std::cout << std::endl;
        PrintSeparator('-', 60);

        auto results = suite.Run();

        int passed = 0;
        int failed = 0;
        double suiteTime = 0.0;

        // Display test results
        for (const auto& result : results) {
            suiteTime += result.DurationMs;

            if (result.Passed) {
                passed++;
                if (options.Verbose) {
                    std::cout << "  ✅ " << result.Name;
                    std::cout << std::string(std::max(1, 50 - (int)result.Name.length()), ' ');
                    PrintSuccess("PASSED");
                    std::cout << " (" << std::fixed << std::setprecision(2) << result.DurationMs
                              << " ms)" << std::endl;
                }
            } else {
                failed++;
                failedTests.push_back(suiteName + "::" + result.Name);
                std::cout << "  ❌ " << result.Name;
                std::cout << std::string(std::max(1, 50 - (int)result.Name.length()), ' ');
                PrintError("FAILED");
                std::cout << " (" << std::fixed << std::setprecision(2) << result.DurationMs
                          << " ms)" << std::endl;
                std::cout << "     📍 " << result.ErrorMessage << std::endl;
            }
        }

        totalPassed += passed;
        totalFailed += failed;
        totalTime += suiteTime;

        // Suite summary
        std::cout << std::endl;
        std::cout << "  📊 Suite Summary: ";
        PrintSuccess(std::to_string(passed) + " passed");
        if (failed > 0) {
            std::cout << ", ";
            PrintError(std::to_string(failed) + " failed");
        }
        std::cout << " (⏱️  " << std::fixed << std::setprecision(2) << suiteTime << " ms)"
                  << std::endl;
    }

    // Final summary
    std::cout << std::endl;
    PrintSeparator('=', 60);
    std::cout << std::endl;

    PrintBold("📈 Final Results");
    std::cout << std::endl;
    PrintSeparator('-', 60);

    std::cout << "  Suites run:   " << suitesRun << " / " << registrations.size() << std::endl;
    std::cout << "  Total tests:  " << (totalPassed + totalFailed) << std::endl;
    PrintSuccess("  ✅ Passed:     " + std::to_string(totalPassed));
    std::cout << std::endl;
    if (totalFailed > 0) {
        PrintError("  ❌ Failed:     " + std::to_string(totalFailed));
        std::cout << std::endl;
    }
    PrintInfo("  ⏱️  Total time:  " + std::to_string(totalTime) + " ms");
    std::cout << std::endl;

    PrintSeparator('-', 60);

    // Failed tests list
    if (totalFailed > 0) {
        std::cout << std::endl;
        PrintError("❌ Failed Tests:");
        for (const auto& test : failedTests) {
            std::cout << "  • " << test << std::endl;
        }
        std::cout << std::endl;
    }

    // Pass rate
    double passRate =
        (totalPassed + totalFailed) > 0 ? (totalPassed * 100.0 / (totalPassed + totalFailed)) : 0.0;

    std::cout << "📊 Pass Rate: ";
    if (passRate == 100.0) {
        PrintSuccess("100%");
    } else if (passRate >= 80.0) {
        PrintWarning(std::to_string(passRate) + "%");
    } else {
        PrintError(std::to_string(passRate) + "%");
    }
    std::cout << std::endl;

    PrintSeparator('=', 60);
    std::cout << std::endl;

    // Exit code
    if (totalFailed == 0) {
        PrintSuccess("✨ All tests passed!");
        std::cout << std::endl;
        return 0;
    } else {
        PrintError("💥 Some tests failed!");
        std::cout << std::endl;
        return 1;
    }
}