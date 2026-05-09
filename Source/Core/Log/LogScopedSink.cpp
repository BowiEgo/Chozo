#include "LogScopedSink.h"
#include "Logger.h"

namespace CZ {

struct FLogScopedSink::Impl {
    Logger::SinkHandle Sink;
};

FLogScopedSink::FLogScopedSink(LogCallback callback, const std::string& pattern)
    : m_Impl(std::make_unique<Impl>()) {
    m_Impl->Sink = Logger::AddCallbackSink(std::move(callback), pattern);
}

FLogScopedSink::~FLogScopedSink() {
    if (m_Impl) {
        Logger::RemoveSink(m_Impl->Sink);
    }
}
} // namespace CZ
