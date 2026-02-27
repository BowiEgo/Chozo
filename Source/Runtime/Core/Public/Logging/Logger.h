#pragma once

#include "LogMacros.h"
#include "LogUtils.h"

#include <functional>
#include <mutex>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <string>

template <typename Mutex> class LogCallbackSink : public spdlog::sinks::base_sink<Mutex> {
public:
    using LogCallback = std::function<void(const std::string&, ELogVerbosity)>;
    explicit LogCallbackSink(LogCallback callback) : m_Callback(std::move(callback)) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        if (!m_Callback) return;

        // Format the message using the sink's formatter
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        // Invoke the UI-side callback
        m_Callback(fmt::to_string(formatted), ChozoUtils::Log::FromSpdlogLevel(msg.level));
    }

    void flush_() override {}

private:
    LogCallback m_Callback;
};

using LogCallbackSink_mt = LogCallbackSink<std::mutex>;

class CORE_API FLogger {
public:
    static FLogger& Get();
    static spdlog::sink_ptr AddCallbackSink(LogCallbackSink_mt::LogCallback callback,
                                            const std::string& pattern);
    static void RemoveSink(spdlog::sink_ptr SinkHandle);

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

#if defined(__clang__)
    #define CZ_DISABLE_TAUTOLOGICAL_WARNING                                                        \
        _Pragma("clang diagnostic push")                                                           \
            _Pragma("clang diagnostic ignored \"-Wtautological-compare\"")
    #define CZ_ENABLE_TAUTOLOGICAL_WARNING _Pragma("clang diagnostic pop")
#else
    #define CZ_DISABLE_TAUTOLOGICAL_WARNING
    #define CZ_ENABLE_TAUTOLOGICAL_WARNING
#endif

#define CZ_LOG(Category, Verbosity, Format, ...)                                                   \
    do {                                                                                           \
        CZ_DISABLE_TAUTOLOGICAL_WARNING                                                            \
        if (static_cast<uint8_t>(ELogVerbosity::Verbosity) <=                                      \
            static_cast<uint8_t>(CZ_LOG_LEVEL)) {                                                  \
            FLogger::Get().Log(#Category, ELogVerbosity::Verbosity, Format, ##__VA_ARGS__);        \
            if (ELogVerbosity::Verbosity == ELogVerbosity::Fatal) {                                \
                CZ_DEBUGBREAK();                                                                   \
            }                                                                                      \
        }                                                                                          \
        CZ_ENABLE_TAUTOLOGICAL_WARNING                                                             \
    } while (0)

#define CZ_CORE_LOG(Verbosity, Format, ...) CZ_LOG(Core, Verbosity, Format, ##__VA_ARGS__)

#define CZ_APP_LOG(Verbosity, Format, ...) CZ_LOG(App, Verbosity, Format, ##__VA_ARGS__)