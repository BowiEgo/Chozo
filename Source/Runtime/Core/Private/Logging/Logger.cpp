#include "Logger.h"

namespace Chozo {

    // Pimpl structure to hide spdlog headers from Public API
    struct FLogger::FImpl {
        std::shared_ptr<spdlog::logger> SpdLogger;
    };

    FLogger& FLogger::Get() {
        static FLogger Instance;
        return Instance;
    }

    FLogger::FLogger() {
        Impl = std::make_unique<FImpl>();

        // Create a multi-threaded color stdout sink
        auto ConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        // Pattern: [%Time] [%Category] [%Level]: %Message
        ConsoleSink->set_pattern("%^[%T] [%n] [%l]: %v%$");

        Impl->SpdLogger = std::make_shared<spdlog::logger>("Chozo", ConsoleSink);
        Impl->SpdLogger->set_level(spdlog::level::trace);
    }

    FLogger::~FLogger() = default;

    void FLogger::LogInternal(const std::string& Category, ELogVerbosity Verbosity, const std::string& Message)
    {
        spdlog::level::level_enum SpdLevel;
        switch (Verbosity) {
            case ELogVerbosity::Trace:   SpdLevel = spdlog::level::trace; break;
            case ELogVerbosity::Info:    SpdLevel = spdlog::level::info; break;
            case ELogVerbosity::Warning: SpdLevel = spdlog::level::warn; break;
            case ELogVerbosity::Error:   SpdLevel = spdlog::level::err; break;
            case ELogVerbosity::Fatal:   SpdLevel = spdlog::level::critical; break;
            default:                     SpdLevel = spdlog::level::info; break;
        }

        // 组装格式：[Category] Message
        std::string FormattedMsg = "[" + Category + "] " + Message;

        Impl->SpdLogger->log(SpdLevel, FormattedMsg);

        if (Verbosity == ELogVerbosity::Fatal) {
            Impl->SpdLogger->flush(); 
            abort(); 
        }
    }
}