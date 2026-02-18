#pragma once

#include "LogMacros.h"

#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class CORE_API FLogger {
public:
    static FLogger& Get();

    /**
     * The core log function that dispatches messages to sinks
     */
    template <typename... Args>
    void Log(const std::string& Category, ELogVerbosity Verbosity, const std::string& Format,
             Args&&... args) {
        std::string FormattedMsg =
            spdlog::fmt_lib::format(fmt::runtime(Format), std::forward<Args>(args)...);
        LogInternal(Category, Verbosity, FormattedMsg);
    }

private:
    FLogger();
    ~FLogger();

    // Forward declaration of internal spdlog implementation (Pimpl pattern)
    struct FImpl;
    std::unique_ptr<FImpl> Impl;

    void LogInternal(const std::string& Category, ELogVerbosity Verbosity,
                     const std::string& Message);
};

#define CZ_LOG(Category, Verbosity, Format, ...)                                                   \
    do {                                                                                           \
        if (ELogVerbosity::Verbosity <= CZ_LOG_LEVEL) {                                            \
            FLogger::Get().Log(#Category, ELogVerbosity::Verbosity, Format, ##__VA_ARGS__);        \
            if (ELogVerbosity::Verbosity == ELogVerbosity::Fatal) {                                \
                CZ_DEBUGBREAK();                                                                   \
            }                                                                                      \
        }                                                                                          \
    } while (0)

#define CZ_CORE_LOG(Verbosity, Format, ...) CZ_LOG(Core, Verbosity, Format, ##__VA_ARGS__)

#define CZ_APP_LOG(Verbosity, Format, ...) CZ_LOG(App, Verbosity, Format, ##__VA_ARGS__)