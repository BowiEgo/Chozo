#pragma once

#include "LogMacros.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Chozo {

    class CORE_API FLogger {
    public:
        static FLogger& Get();

        /**
         * The core log function that dispatches messages to sinks
         */
        template<typename... Args>
        void Log(const std::string& Category, ELogVerbosity Verbosity, const std::string& Format, Args&&... args)
        {
            std::string FormattedMsg = spdlog::fmt_lib::format(fmt::runtime(Format), std::forward<Args>(args)...);
            LogInternal(Category, Verbosity, FormattedMsg);
        }

    private:
        FLogger();
        ~FLogger();

        // Forward declaration of internal spdlog implementation (Pimpl pattern)
        struct FImpl;
        std::unique_ptr<FImpl> Impl;

        void LogInternal(const std::string& Category, ELogVerbosity Verbosity, const std::string& Message);
    };
}

#define CZ_LOG(Category, Verbosity, Format, ...) \
    Chozo::FLogger::Get().Log(#Category, Chozo::ELogVerbosity::Verbosity, Format, ##__VA_ARGS__)