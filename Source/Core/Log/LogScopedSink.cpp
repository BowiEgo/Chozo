#include "LogScopedSink.h"
#include "Logger.h"

#include <spdlog/spdlog.h>

namespace CZ {

struct FLogScopedSink::Impl {
    spdlog::sink_ptr Sink;
};

FLogScopedSink::FLogScopedSink(LogCallback callback, const std::string& pattern)
    : m_Impl(std::make_unique<Impl>()) {
    // 需要将 LogCallback 适配成 LogCallbackSink_mt::LogCallback（如果类型不同）
    // 这里假设 FLogger::AddCallbackSink 接受 std::function<void(const std::string&, ELogVerbosity)>
    m_Impl->Sink = Logger::AddCallbackSink(std::move(callback), pattern);
}

FLogScopedSink::~FLogScopedSink() {
    if (m_Impl) {
        Logger::RemoveSink(m_Impl->Sink);
    }
}
} // namespace CZ
