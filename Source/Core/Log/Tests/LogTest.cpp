#include <Core/Log/LogMacros.hpp>
#include <Core/Log/Logger.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

using namespace CZ;

namespace {

/// Collects everything a sink receives while it is registered, so a case can look for its own
/// marker message without being confused by log traffic from other suites.
struct CollectedLogs {
    std::mutex Mutex;
    std::vector<std::pair<std::string, LogVerbosity>> Entries;

    void Add(const std::string& message, LogVerbosity verbosity) {
        std::lock_guard<std::mutex> lock(Mutex);
        Entries.emplace_back(message, verbosity);
    }

    bool Contains(const std::string& needle, LogVerbosity verbosity) {
        std::lock_guard<std::mutex> lock(Mutex);
        return std::any_of(Entries.begin(), Entries.end(), [&](const auto& entry) {
            return entry.first.find(needle) != std::string::npos && entry.second == verbosity;
        });
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(Mutex);
        return Entries.size();
    }
};

} // namespace

TEST_SUITE("Logger") {

    TEST_CASE("Callback sinks receive the formatted message and its verbosity") {
        CollectedLogs collected;
        auto sink = Logger::AddCallbackSink(
            [&](const std::string& message, LogVerbosity verbosity) {
                collected.Add(message, verbosity);
            },
            "%v");

        Logger::Get().Log("UnitTest", LogVerbosity::Warning, "cze-marker-warning");
        Logger::RemoveSink(sink);

        CHECK(collected.Contains("cze-marker-warning", LogVerbosity::Warning));
    }

    TEST_CASE("Verbosity is preserved for every level below Fatal") {
        CollectedLogs collected;
        auto sink = Logger::AddCallbackSink(
            [&](const std::string& message, LogVerbosity verbosity) {
                collected.Add(message, verbosity);
            },
            "%v");

        Logger::Get().Log("UnitTest", LogVerbosity::Error, "cze-marker-error");
        Logger::Get().Log("UnitTest", LogVerbosity::Info, "cze-marker-info");
        Logger::Get().Log("UnitTest", LogVerbosity::Debug, "cze-marker-debug");
        Logger::Get().Log("UnitTest", LogVerbosity::Trace, "cze-marker-trace");
        Logger::RemoveSink(sink);

        CHECK(collected.Contains("cze-marker-error", LogVerbosity::Error));
        CHECK(collected.Contains("cze-marker-info", LogVerbosity::Info));
        CHECK(collected.Contains("cze-marker-debug", LogVerbosity::Debug));
        CHECK(collected.Contains("cze-marker-trace", LogVerbosity::Trace));
    }

    TEST_CASE("A removed sink stops receiving messages") {
        CollectedLogs collected;
        auto sink = Logger::AddCallbackSink(
            [&](const std::string& message, LogVerbosity verbosity) {
                collected.Add(message, verbosity);
            },
            "%v");

        Logger::Get().Log("UnitTest", LogVerbosity::Info, "cze-marker-before-removal");
        Logger::RemoveSink(sink);

        const size_t afterRemoval = collected.Size();
        Logger::Get().Log("UnitTest", LogVerbosity::Info, "cze-marker-after-removal");

        CHECK(collected.Contains("cze-marker-before-removal", LogVerbosity::Info));
        CHECK_FALSE(collected.Contains("cze-marker-after-removal", LogVerbosity::Info));
        CHECK_EQ(collected.Size(), afterRemoval);
    }

    TEST_CASE("The pattern controls the formatted payload") {
        CollectedLogs collected;
        auto sink = Logger::AddCallbackSink(
            [&](const std::string& message, LogVerbosity verbosity) {
                collected.Add(message, verbosity);
            },
            "cze-prefix|%v");

        Logger::Get().Log("UnitTest", LogVerbosity::Info, "cze-marker-pattern");
        Logger::RemoveSink(sink);

        // The marker prefix proves the pattern was applied; the exact %v expansion depends on
        // the log format and is not asserted here.
        CHECK(collected.Contains("cze-prefix|", LogVerbosity::Info));
        CHECK(collected.Contains("cze-marker-pattern", LogVerbosity::Info));
    }

    TEST_CASE("Log macros route through the same logger") {
        CollectedLogs collected;
        auto sink = Logger::AddCallbackSink(
            [&](const std::string& message, LogVerbosity verbosity) {
                collected.Add(message, verbosity);
            },
            "%v");

        CZ_CORE_LOG(Info, "cze-marker-macro {}", 42);
        Logger::RemoveSink(sink);

        CHECK(collected.Contains("cze-marker-macro 42", LogVerbosity::Info));
    }
}
