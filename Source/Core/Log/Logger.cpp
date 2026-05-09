#include "Core/Log/Logger.h"

#include "LogUtils.h"

#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace CZ {

template <typename Mutex> class LogCallbackSinkImpl : public spdlog::sinks::base_sink<Mutex> {
public:
    using LogCallback = std::function<void(const std::string&, LogVerbosity)>;
    explicit LogCallbackSinkImpl(LogCallback callback) : m_Callback(std::move(callback)) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        if (!m_Callback) return;
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        m_Callback(fmt::to_string(formatted), LogUtils::FromSpdlogLevel(msg.level));
    }

    void flush_() override {}

private:
    LogCallback m_Callback;
};

using LogCallbackSink_mt = LogCallbackSinkImpl<std::mutex>;

// Pimpl structure to hide spdlog headers from Public API
struct Logger::FImpl {
    std::shared_ptr<spdlog::logger> SpdLogger;
    const std::string DefaultPattern =
        "%^[%T] [%n] [%l]: %v%$"; // Pattern: [%Time] [%Category] [%Level]: %Message
};

Logger& Logger::Get() {
    static Logger Instance;
    return Instance;
}

Logger::Logger() {
    Impl = std::make_unique<FImpl>();

    // Create a multi-threaded color stdout sink
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern(Impl->DefaultPattern);

    Impl->SpdLogger = std::make_shared<spdlog::logger>("Chozo", consoleSink);
    Impl->SpdLogger->set_level(spdlog::level::trace);
}

Logger::~Logger() = default;

Logger::SinkHandle Logger::AddCallbackSink(LogCallbackSink_mt::LogCallback callback,
                                           const std::string& pattern) {
    if (!callback) return nullptr; // Avoid registering null callbacks to prevent crashes

    auto& logger = Logger::Get();
    auto& sinks  = logger.Impl->SpdLogger->sinks();

    auto newSink = std::make_shared<LogCallbackSink_mt>(std::move(callback));
    newSink->set_pattern(pattern);
    sinks.push_back(newSink);

    return newSink;
}

void Logger::RemoveSink(Logger::SinkHandle sinkHandle) {
    if (!sinkHandle) return;

    auto& logger = Logger::Get();
    auto& sinks  = logger.Impl->SpdLogger->sinks();

    // Standard way to remove a specific element from a vector.
    // This is much safer than clearing everything.
    sinks.erase(std::remove(sinks.begin(), sinks.end(), sinkHandle), sinks.end());
}

void Logger::Log(const std::string& category, LogVerbosity verbosity, const std::string& message) {
    auto spdLevel = LogUtils::ToSpdlogLevel(verbosity);

    Impl->SpdLogger->log(spdLevel, "[{}] {}", category, message);

    if (verbosity == LogVerbosity::Fatal) {
        Impl->SpdLogger->flush();
        CZ_DEBUGBREAK();
        abort();
    }
}

} // namespace CZ
