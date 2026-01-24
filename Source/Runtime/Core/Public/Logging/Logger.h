#pragma once

#include "LogMacros.h"

namespace Chozo {

    class CORE_API FLogger {
    public:
        static FLogger& Get();

        /**
         * The core log function that dispatches messages to sinks
         */
        void Log(const std::string& Category, ELogVerbosity Verbosity, const std::string& Message);

    private:
        FLogger();
        ~FLogger();

        // Forward declaration of internal spdlog implementation (Pimpl pattern)
        struct FImpl;
        std::unique_ptr<FImpl> Impl;
    };
}

/**
 * Universal log macro
 */
#define CZ_LOG(Category, Verbosity, Message) \
    Chozo::FLogger::Get().Log(#Category, Chozo::ELogVerbosity::Verbosity, Message)