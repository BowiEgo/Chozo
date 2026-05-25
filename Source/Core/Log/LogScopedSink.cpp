#include <Core/Log/LogScopedSink.hpp>
#include <Core/Log/Logger.hpp>

namespace CZ {

struct LogScopedSink::Impl {
    Logger::SinkHandle Sink;
};

LogScopedSink::LogScopedSink(LogCallback callback, const std::string& pattern)
    : m_Impl(std::make_unique<Impl>()) {
    m_Impl->Sink = Logger::AddCallbackSink(std::move(callback), pattern);
}

LogScopedSink::~LogScopedSink() {
    if (m_Impl) {
        Logger::RemoveSink(m_Impl->Sink);
    }
}
} // namespace CZ
