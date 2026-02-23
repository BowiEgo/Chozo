#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

// Pimpl structure to hide spdlog headers from Public API
struct FLogger::FImpl {
    std::shared_ptr<spdlog::logger> SpdLogger;
    const std::string DefaultPattern =
        "%^[%T] [%n] [%l]: %v%$"; // Pattern: [%Time] [%Category] [%Level]: %Message
};

FLogger& FLogger::Get() {
    static FLogger Instance;
    return Instance;
}

spdlog::sink_ptr FLogger::AddCallbackSink(LogCallbackSink_mt::LogCallback callback,
                                          const std::string& pattern) {
    if (!callback) return nullptr; // Avoid registering null callbacks to prevent crashes

    auto& logger = FLogger::Get();
    auto& sinks = logger.Impl->SpdLogger->sinks();

    auto newSink = std::make_shared<LogCallbackSink_mt>(std::move(callback));
    newSink->set_pattern(pattern);
    sinks.push_back(newSink);

    return newSink;
}

void FLogger::RemoveSink(spdlog::sink_ptr sinkHandle) {
    if (!sinkHandle) return;

    auto& logger = FLogger::Get();
    auto& sinks = logger.Impl->SpdLogger->sinks();

    // Standard way to remove a specific element from a vector.
    // This is much safer than clearing everything.
    sinks.erase(std::remove(sinks.begin(), sinks.end(), sinkHandle), sinks.end());
}

FLogger::FLogger() {
    Impl = std::make_unique<FImpl>();

    // Create a multi-threaded color stdout sink
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern(Impl->DefaultPattern);

    Impl->SpdLogger = std::make_shared<spdlog::logger>("Chozo", consoleSink);
    Impl->SpdLogger->set_level(spdlog::level::trace);
}

FLogger::~FLogger() = default;

void FLogger::LogInternal(const std::string& category, ELogVerbosity verbosity,
                          const std::string& message) {
    auto spdLevel = ChozoUtils::Log::ToSpdlogLevel(verbosity);

    Impl->SpdLogger->log(spdLevel, "[{}] {}", category, message);

    if (verbosity == ELogVerbosity::Fatal) {
        Impl->SpdLogger->flush();
        CZ_DEBUGBREAK();
        abort();
    }
}