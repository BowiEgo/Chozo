#include "Logger.h"

// Pimpl structure to hide spdlog headers from Public API
struct FLogger::FImpl {
    std::shared_ptr<spdlog::logger> SpdLogger;
};

FLogger &FLogger::Get() {
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

void FLogger::LogInternal(const std::string &Category, ELogVerbosity Verbosity,
                          const std::string &Message) {
    static constexpr spdlog::level::level_enum SpdLevels[] = {
        spdlog::level::critical, // 0: Fatal
        spdlog::level::err,      // 1: Error
        spdlog::level::warn,     // 2: Warning
        spdlog::level::info,     // 3: Info
        spdlog::level::debug,    // 4: Debug
        spdlog::level::trace     // 5: Trace
    };
    auto SpdLevel = SpdLevels[static_cast<uint32>(Verbosity)];

    Impl->SpdLogger->log(SpdLevel, "[{}] {}", Category, Message);

    if (Verbosity == ELogVerbosity::Fatal) {
        Impl->SpdLogger->flush();
        CZ_DEBUGBREAK();
        abort();
    }
}